#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <map>
#include <utility>
#include <vector>
#include <mpi.h>
#include "error.h"
#include "mpi.h"
#include "grain.h"
#include "wclock.h"
#include "utils.h"

struct grain_info {
    size_t count;
    size_t *sizes;
};

extern "C" {

void init_grain_info(struct grain_info *gi)
{
    gi->sizes = (size_t *)malloc(gi->count * sizeof(*gi->sizes));
}

/* Global grain distribution */
struct grain_dist {
    /* Global grain distribution array
       ProcessIndex -> LocalGrainIndex -> GlobalGrainIndex */
    std::vector<std::vector<int> > _gd_vec;

    /* Same as _gd_vec but borrows the subvectors as raw pointers
       ProcessIndex -> LocalGrainIndex -> GlobalGrainIndex */
    std::vector<int *> _gd;

    /* GrainIndex -> GrainSize */
    std::vector<size_t> _gd_counts;
};

void initialize_grains(struct grain_info *gi,
                       int num_procs,
                       struct grain_dist *gd)
{
    assert(num_procs > 0); /* otherwise how do we even exist?? */

    gd->_gd_vec.resize(num_procs);

    /* MultiMap<Load, ProcessIndex> */
    std::multimap<size_t, int> proc_load;
    for (int p = 0; p < num_procs; ++p) {
        proc_load.insert(std::make_pair(0, p));
    }
    // FIXME: here we assume the grains are already sorted by size in
    // ascending order (load balancing may suffer otherwise)

    // this simple algorithm makes intuitive sense and turns out to have a
    // name too: longest processing time (LPT) algorithm
    for (size_t g = gi->count; g--;) {
        const auto ipl = proc_load.begin();
        std::pair<size_t, int> pl = *ipl;
        const int p = pl.second;
        proc_load.erase(ipl);
        pl.first += gi->sizes[g];
        proc_load.insert(std::move(pl));
        // gi->map[g] = p;
        gd->_gd_vec[p].push_back(g);
    }

    gd->_gd.resize(num_procs);
    gd->_gd_counts.resize(num_procs);
    for (int p = 0; p < num_procs; ++p) {
        gd->_gd[p] = gd->_gd_vec[p].data();
        gd->_gd_counts[p] = gd->_gd_vec[p].size();
    }
}

size_t scatter_grain_info(int **lgd,
                          const int *const *pd,
                          const size_t *pd_counts)
{
    int p;
    size_t lgcount;
    void *vlgd;
    const void **vpd = NULL;
    if (mpi.rank == 0) {
        vpd = (const void **)xmalloc(mpi.size * sizeof(*vpd));
        for (p = 0; p < mpi.size; ++p) {
            vpd[p] = pd[p];
        }
    }
    xtry(varscatter(vpd, pd_counts, &vlgd, &lgcount, MPI_INT,
                    0, MPI_COMM_WORLD));
    free(vpd);
    *lgd = (int *)vlgd;
    return lgcount;
}

void **alloc_grains(struct grain_info *gi, int *lgd, int lgcount)
{
    void **grains = (void **)xmalloc(lgcount * sizeof(*grains));
    for (int i = 0; i != lgcount; ++i) {
        grains[i] = xmalloc(gi->sizes[lgd[i]]);
    }
    return grains;
}

void free_grain_info(struct grain_info *gi)
{
    free(gi->sizes);
//    free(gi->map);
}

// this is not very efficiently written ...
std::vector<std::vector<int> >
rescatter_grains(int *gd_size_matrix, int gcount, int num_procs)
{

    /* MultiMap<Load, ProcessIndex> */
    std::multimap<size_t, int> proc_load;
    for (int p = 0; p < num_procs; ++p) {
        proc_load.insert(std::make_pair(0, p));
    }

    /* ProcessIndex -> MultiMap<Size, GrainIndex> */
    std::vector<std::multimap<size_t, int> > mpg(num_procs);
    std::vector<std::multimap<size_t, int>::iterator> rmpg(gcount * num_procs);
    for (int p = 0; p < num_procs; ++p) {
        for (int g = 0; g < gcount; ++g) {
            const size_t m = gd_size_matrix[p * gcount + g];
            rmpg[p * gcount + g] = mpg[p].insert(std::make_pair(m, g));
        }
    }

    // load balancing vs minimizing transfer I think loading balancing is more
    // critical so we will center the algorithm on distributing the grains as
    // evenly as possible as we did during initialization. however we can give
    // some 'leeway': instead of always picking the SMALLEST node, we choose
    // among the N smallest nodes that minimizes the data transfer
    static const double LOAD_THRESHOLD = 0.2;
    std::vector<std::vector<int> > pd_vec(num_procs);
    while (!mpg[0].empty()) {
        auto best_ipl = proc_load.begin();
        const size_t load0 = best_ipl->first;
        auto best_impg = mpg[best_ipl->second].begin();
        for (auto ipl = ++proc_load.begin(), ipl_end = proc_load.end();
             ipl != ipl_end && ipl->first <= load0 * (1 + LOAD_THRESHOLD);
             ++ipl) {
            const auto impg = mpg[ipl->second].begin();
            if (impg->first > best_impg->first) {
                best_impg = impg;
                best_ipl = ipl;
            }
        }

        const size_t best_g = best_impg->second;
        size_t total_grain_size = 0;
        for (int p = 0; p < num_procs; ++p) {
            total_grain_size += gd_size_matrix[p * gcount + best_g];
            mpg[p].erase(rmpg[p * gcount + best_g]);
        }

        const size_t best_p = best_ipl->second;
        const size_t new_load = best_ipl->first + total_grain_size;
        proc_load.erase(best_ipl);
        proc_load.insert(std::make_pair(new_load, best_p));

        pd_vec[best_p].push_back(best_g);
    }

//    for (const auto &ipl : proc_load) {
//        printf("%i: after/load=%zu\n", ipl.second, ipl.first);
//    }

    return pd_vec;
}

struct jaggedii {
    int *data; /* [size] + [offset] * n + [values] */
};

static inline
struct jaggedii new_jaggedii(const std::vector<std::vector<int> > &v)
{
    struct jaggedii m;
    size_t subsize = 0;
    for (const auto &vi : v) {
        subsize += vi.size();
    }
    m.data = (int *)xmalloc((1 + v.size() + subsize) * sizeof(*m.data));
    m.data[0] = v.size();
    subsize = 0;
    for (size_t i = 0; i < v.size(); ++i) {
        memcpy(m.data + 1 + v.size() + subsize, v[i].data(),
               v[i].size() * sizeof(*m.data));
        subsize += v[i].size();
        m.data[1 + i] = subsize;
    }
    return m;
}

static inline
void free_jaggedii(struct jaggedii m)
{
    free(m.data);
}

static inline
size_t get_jaggedii_count1(struct jaggedii m)
{
    assert(m.data);
    return m.data[0];
}

static inline
size_t get_jaggedii_count2(struct jaggedii m, size_t i)
{
    assert(i < get_jaggedii_count1(m));
    return m.data[i + 1] - (i && m.data[i]);
}

static inline
size_t get_jaggedii_count_all(struct jaggedii m)
{
    return 1 + get_jaggedii_count1(m) + m.data[get_jaggedii_count1(m)];
}

static inline
int *get_jaggedii_elem(struct jaggedii m, size_t i, size_t j)
{
    assert(j < get_jaggedii_count2(m, i));
    return &m.data[1 + get_jaggedii_count1(m) + (i && m.data[i])];
}

void dump_vectorii(const char *prefix,
                   const std::vector<std::vector<int> > &v)
{
    for (size_t i = 0; i < v.size(); ++i) {
        for (size_t j = 0; j < v[i].size(); ++j) {
            printf("%s: [%zu][%zu] = %i\n", prefix, i, j,
                   v[i][j]);
        }
    }
}


void dump_jaggedii(const char *prefix, struct jaggedii m)
{
    printf("%s: [ \n", prefix);
    for (size_t i = 0; i < get_jaggedii_count_all(m); ++i) {
        printf("%i ", m.data[i]);
    }
    printf("]\n");
}

void cpp_main(int newgcount)
{

    /* example data */
    struct grain_info gi;
    gi.count = newgcount;
    init_grain_info(&gi);
    for (size_t g = 0; g != gi.count; ++g) {
        gi.sizes[g] = g * g * sizeof(double); // FIXME: hard-coded
    }

    /* local grain count (number of local grains indices) */
    int lgcount;
    /* local grain distribution (LocalGrainIndex -> GrainIndex) */
    int *lgd;
    /* local grain data (LocalGrainIndex -> GrainData) */
    void **lgrains;

    struct grain_dist gd;
    if (mpi.rank == 0) {
        initialize_grains(&gi, mpi.size, &gd);
    }

    double t;

    t = get_gwclock();
    lgcount = scatter_grain_info(&lgd, gd._gd.data(), gd._gd_counts.data());
    printf0("time_scatter=%.17g\n", get_gwclock() - t);

    t = get_gwclock();
    lgrains = alloc_grains(&gi, lgd, lgcount);
    printf0("time_allocgrains=%.17g\n", get_gwclock() - t);

    /* print some debug info */
    size_t load = 0;
    for (int lg = 0; lg < lgcount; ++lg) {
        int g = lgd[lg];
//        printf("%i: grain=%i size=%zu\n", mpi.rank, g, gi.sizes[g]);
        load += gi.sizes[g];
    }
    printf("%i: before/load=%zu\n", mpi.rank, load / 8);

    t = get_gwclock();
    /* fill with some data */
    for (int lg = 0; lg < lgcount; ++lg) {
        const int g = lgd[lg];
        double *const grain = (double *)lgrains[lg];
        const size_t n = gi.sizes[g] / sizeof(*grain);
        for (size_t i = 0; i < n; ++i) {
            grain[i] = g * 100 + i;
            // printf("%i: [%zu] = %f\n", mpi.rank, i, grain[i]);
        }
    }
    printf0("time_filldata=%.17g\n", get_gwclock() - t);

    t = get_gwclock();
    /* split up the data */
    std::vector<std::vector<double> > newgrains(newgcount);
    std::vector<std::vector<int> > newgrains_indices(newgcount);
    for (int lg = 0; lg < lgcount; ++lg) {
        const int g = lgd[lg];
        double *const grain = (double *)lgrains[lg];
        const size_t n = gi.sizes[g] / sizeof(*grain);
        for (size_t i = 0; i < n; ++i) {
            const int newg = rand() % newgcount;
            const int newi = i; // FIXME: use real index
            newgrains[newg].push_back(grain[i]);
            newgrains_indices[newg].push_back(newi);
            // printf("%i: %i[%zu] -> %i[%i]\n", mpi.rank, g, i, newg, newi);
        }
    }

    /* query root for the new distribution */
    /* GrainIndex -> GrainSize */
    std::vector<int> new_gd_sizes(newgcount);
    for (int newg = 0; newg < newgcount; ++newg) {
        new_gd_sizes[newg] = newgrains[newg].size();
    }
    // gd_size_matrix[ProcessIndex * GrainCount + GrainIndex] = GrainSize
    std::vector<int> gd_size_matrix;
    if (mpi.rank == 0) {
        gd_size_matrix.resize(newgcount * mpi.size);
    }
    xtry(MPI_Gather(new_gd_sizes.data(), newgcount, MPI_INT,
                    gd_size_matrix.data(), newgcount, MPI_INT,
                    0, MPI_COMM_WORLD));
    /* ProcessIndex -> LocalGrainIndex -> GlobalGrainIndex */
    struct jaggedii jpd;
    int jsize;
    if (mpi.rank == 0) {
        std::vector<std::vector<int> > new_pd_vec;
        new_pd_vec = rescatter_grains(gd_size_matrix.data(),
                                      newgcount, mpi.size);
        jpd = new_jaggedii(new_pd_vec);
        jsize = get_jaggedii_count_all(jpd);
    }
    xtry(MPI_Bcast(&jsize, 1, MPI_INT, 0, MPI_COMM_WORLD));
    if (mpi.rank != 0) {
        jpd.data = (int *)xmalloc(jsize * sizeof(*jpd.data));
    }
    xtry(MPI_Bcast(jpd.data, jsize, MPI_INT, 0, MPI_COMM_WORLD));

    /* redistribute */
    size_t total_pack_size = 0;
    std::vector<size_t> pack_size(mpi.size);
    for (int p = 0; p < mpi.size; ++p) {
        pack_size[p] = get_pack_size(get_jaggedii_count2(jpd, p),
                                     MPI_INT, MPI_COMM_WORLD);
        const int nlg = get_jaggedii_count2(jpd, p);
        for (int lg = 0; lg < nlg; ++lg) {
            const int g = *get_jaggedii_elem(jpd, p, lg);
            const size_t gcount = new_gd_sizes[g];
            pack_size[p] +=
                get_pack_size(gcount, MPI_INT, MPI_COMM_WORLD) +
                get_pack_size(gcount, MPI_DOUBLE, MPI_COMM_WORLD);
        }
        total_pack_size += pack_size[p];
    }
    std::vector<char> packbuf(total_pack_size);
    char *packptr = packbuf.data();
    std::vector<MPI_Request> reqs;
    for (int p = 0; p < mpi.size; ++p) {
        int packsize = pack_size[p], pos = 0;
        const int nlg = get_jaggedii_count2(jpd, p);
        std::vector<int> lgdoffsets(nlg + 1);
        for (int lg = 0; lg < nlg; ++lg) {
            const int g = *get_jaggedii_elem(jpd, p, lg);
            lgdoffsets[lg + 1] = lgdoffsets[lg] + new_gd_sizes[g];
        }
        xtry(MPI_Pack(lgdoffsets.data() + 1, nlg, MPI_INT,
                      packptr, packsize, &pos, MPI_COMM_WORLD));
        for (int lg = 0; lg < nlg; ++lg) {
            const int g = *get_jaggedii_elem(jpd, p, lg);
            const int gcount = new_gd_sizes[g];
            xtry(MPI_Pack(newgrains_indices[g].data(), gcount, MPI_INT,
                          packptr, packsize, &pos, MPI_COMM_WORLD));
        }
        for (int lg = 0; lg < nlg; ++lg) {
            const int g = *get_jaggedii_elem(jpd, p, lg);
            const int gcount = new_gd_sizes[g];
            xtry(MPI_Pack(newgrains[g].data(), gcount, MPI_DOUBLE,
                          packptr, packsize, &pos, MPI_COMM_WORLD));
        }
        // for (char *i = packptr; i != packptr + pos; ++i) {
        //     printf("%i . %i\n", i - packptr, *i);
        // }
        reqs.push_back(isend_message(packptr, pos, MPI_PACKED,
                                     p, MPI_COMM_WORLD));
        packptr += packsize;
    }
//    printf("@@ %i == %i\n", packptr - packbuf.data(), total_pack_size);
    std::vector<void *> recvbufs(mpi.size);
    std::vector<size_t> recvcounts(mpi.size);
    const int mynlg = get_jaggedii_count2(jpd, mpi.rank);
    for (int p = 0; p < mpi.size; ++p) {
        reqs.push_back(precv_message(&recvbufs[p],
                                     &recvcounts[p],
                                     MPI_PACKED,
                                     p,
                                     MPI_COMM_WORLD));
    }
    // IDEA: this could be interleaved with the unpacking below btw
    xtry(MPI_Waitall(reqs.size(), reqs.data(), MPI_STATUSES_IGNORE));

    for (int p = 0; p < mpi.size; ++p) {
        int packsize = recvcounts[p], pos = 0;
        const void *packptr = recvbufs[p];
        std::vector<int> lgdoffsets(mynlg + 1);
//        printf("mynlg=%i\n", mynlg);
        xtry(MPI_Unpack(packptr, packsize, &pos,
                        lgdoffsets.data() + 1, mynlg,
                        MPI_INT, MPI_COMM_WORLD));
        std::vector<int> recvgrains_indices(lgdoffsets[mynlg]);
        std::vector<double> recvgrains(lgdoffsets[mynlg]);
        for (int lg = 0; lg < mynlg; ++lg) {
            xtry(MPI_Unpack(packptr, packsize, &pos,
                            recvgrains_indices.data() + lgdoffsets[lg],
                            lgdoffsets[lg + 1] - lgdoffsets[lg],
                            MPI_INT, MPI_COMM_WORLD));
        }
        for (int lg = 0; lg < mynlg; ++lg) {
            xtry(MPI_Unpack(packptr, packsize, &pos,
                            recvgrains.data() + lgdoffsets[lg],
                            lgdoffsets[lg + 1] - lgdoffsets[lg],
                            MPI_DOUBLE, MPI_COMM_WORLD));
        }
    }
    printf0("time_transpose=%.17g\n", get_gwclock() - t);

    for (int p = 0; p < mpi.size; ++p) {
        free(recvbufs[p]);
    }

    free_grain_info(&gi);
}

}
