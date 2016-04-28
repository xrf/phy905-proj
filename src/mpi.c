#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include "benchmark.h"
#include "error.h"
#include "string.h"
#include "mpi.h"

struct mpi mpi;

void init_mpi(int *argc, char ***argv)
{
    int provided, required = MPI_THREAD_FUNNELED;
    xtry(MPI_Init_thread(argc, argv, required, &provided));
    xensure(provided >= required);
    xtry(MPI_Comm_rank(MPI_COMM_WORLD, &mpi.rank));
    xtry(MPI_Comm_size(MPI_COMM_WORLD, &mpi.size));
}

MPI_Request precv_message(void **out_buf,
                          size_t *out_count,
                          MPI_Datatype type,
                          int src,
                          MPI_Comm comm)
{
    int count, size;
    void *buf;
    MPI_Request req;
    MPI_Status st;

    xtry(MPI_Probe(src, 0, comm, &st));
    xtry(MPI_Get_count(&st, type, &count));
    xtry(MPI_Type_size(type, &size));

    buf = xmalloc(count * size);

    req = irecv_message(buf, count, type, src, comm);

    *out_count = count;
    *out_buf = buf;
    return req;
}

int varscatter(const void *const *sendbufs,
               const size_t *sendcounts,
               void **recvbuf,
               size_t *recvcount,
               MPI_Datatype type,
               int root,
               MPI_Comm comm)
{
    int e, size, rank, commsize;

    e = MPI_Type_size(type, &size);
    if (e) {
        return e;
    }

    e = MPI_Comm_rank(comm, &rank);
    if (e) {
        return e;
    }

    e = MPI_Comm_size(comm, &commsize);
    if (e) {
        return e;
    }

    if (rank == root) {
        const size_t count = sendcounts[rank];
        int p;
        void *buf;
        MPI_Request *reqs;

        reqs = (MPI_Request *)xmalloc(commsize * sizeof(*reqs));

        ++reqs;
        for (p = 0; p < commsize; ++p) {
            if (p == rank) {
                --reqs;
                continue;
            }
            xtry(MPI_Isend(sendbufs[p], sendcounts[p], type, p,
                           0, comm, reqs + p));
        }
        buf = xmalloc(count * size);
        memcpy(buf, sendbufs[rank], count * size);
        xtry(MPI_Waitall(commsize - 1, reqs + 1, MPI_STATUSES_IGNORE));

        free(reqs);

        *recvbuf = buf;
        *recvcount = count;

    } else {
        int count;
        void *buf;
        MPI_Status st;

        xtry(MPI_Probe(root, 0, comm, &st));
        xtry(MPI_Get_count(&st, type, &count));

        buf = xmalloc(count * size);

        xtry(MPI_Recv(buf, count, type, root,
                         0, comm, MPI_STATUS_IGNORE));

        *recvbuf = buf;
        *recvcount = (size_t)count;

    }
    return 0;
}

static void broadcast_i(void *root, int *x)
{
    xtry(MPI_Bcast(x, 1, MPI_INT, *(int *)root, MPI_COMM_WORLD));
}

bm *init_parallel_bm(parallel_bm *self, int rank, int root)
{
    bm *bench = parallel_bm_as_bm(self);
    self->_root = root;
    *bench = make_bm();
    set_bm_broadcast_func(bench, &broadcast_i, &self->_root);
    if (rank != root) {
        set_bm_time_func(bench, NULL);
    }
    return bench;
}

bm *parallel_bm_as_bm(parallel_bm *self)
{
    return &self->_bench;
}
