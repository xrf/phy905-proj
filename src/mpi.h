#ifndef G_0OPL73W4MUO995T5NWYK0TGHVE3CV
#define G_0OPL73W4MUO995T5NWYK0TGHVE3CV
#include <stddef.h>
#include <mpi.h>
#include "benchmark.h"
#include "error.h"
#ifdef __cplusplus
extern "C" {
#endif

struct mpi {
    int rank, size;
};

extern struct mpi mpi;

/** Initialize MPI and set the members of the global `mpi` variable. */
void init_mpi(int *argc, char ***argv);

static inline
MPI_Request isend_message(const void *buf,
                          size_t count,
                          MPI_Datatype type,
                          int dst,
                          MPI_Comm comm)
{
    MPI_Request req;
    xtry(MPI_Isend(buf, count, type, dst, 0, comm, &req));
    return req;
}

static inline
MPI_Request irecv_message(void *out_buf,
                          size_t count,
                          MPI_Datatype type,
                          int src,
                          MPI_Comm comm)
{
    MPI_Request req;
    xtry(MPI_Irecv(out_buf, count, type, src, 0, comm, &req));
    return req;
}

/** Receive a message of unknown length into a new buffer allocated using
    `malloc`. */
MPI_Request precv_message(void **out_buf,
                          size_t *out_count,
                          MPI_Datatype type,
                          int src,
                          MPI_Comm comm);

static inline
size_t get_pack_size(size_t count, MPI_Datatype type, MPI_Comm comm)
{
    int size;
    MPI_Pack_size(count, type, comm, &size);
    return size;
}

int varscatter(const void *const *sendbufs,
               const size_t *sendcounts,
               void **recvbuf,
               size_t *recvcount,
               MPI_Datatype type,
               int root,
               MPI_Comm comm);

typedef struct {
    bm _bench;
    int _root;
} parallel_bm;

bm *init_parallel_bm(parallel_bm *, int rank, int root);

bm *parallel_bm_as_bm(parallel_bm *);

#ifdef __cplusplus
}
#endif
#endif
