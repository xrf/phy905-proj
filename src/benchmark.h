#ifndef G_FKNXDZO6LMGAK79V5SQ88ARYHZR7K
#define G_FKNXDZO6LMGAK79V5SQ88ARYHZR7K
#include <stddef.h>
#include "statistics.h"
#ifdef __cplusplus
extern "C" {
#endif

/** Helper function for repeating a calculation until the total time taken is
    at least `preferred_time` seconds.  Note: `i` should be initialized to
    `(size_t)(-1)`. */
int benchmark(size_t *i, double *time, size_t *count, double preferred_time);

/** Similar to `benchmark` but provides more flexibility.  When doing MPI
    benchmarks, `get_time` should be omitted on all but one node, and
    `broadcast` must be set to a non-`NULL` function that broadcasts a single
    `int`. */
int benchmark_with(size_t *i,
                   double *time,
                   size_t *count,
                   double preferred_time,
                   double (*get_time)(void),
                   void (*broadcast)(void *ctx, int *),
                   void *broadcast_ctx);

/** `bm` objects track the state of a particular benchmark.

    Benchmarks are done via a doubly-nested loop:

      - The inner loop repeats the action enough times until the duration
        exceeds `preferred_time`.  The time is computed by dividing this
        duration by the number of repeats.  The number of repeats is
        controlled by `num_subrepeats`, which can be initialized to a certain
        value and will automatically increase itself if the `preferred_time`
        condition is not met.

      - The outer loop repeats the inner loop until a fixed number of repeats
        is complete, as configured by `num_repeats`.  This is used to account
        for noise and fluctuations in the measurements.

*/
typedef struct {
    struct statistics_state stats;
    size_t num_repeats, num_subrepeats, repeat_index, subrepeat_index;
    double time, preferred_time;
    size_t _num_skipped;
    double (*_get_time)(void);
    void (*_broadcast)(void *, int *);
    void *_broadcast_ctx;
} bm;

/** Make a `bm` object. */
bm make_bm(void);

/** Set the function used to obtain the current time in seconds.  When doing
    MPI benchmarks, the function must be set to `NULL` on all but the root
    node.  The default is `&mysecond`. */
void set_bm_time_func(bm *, double (*get_time)(void));

/** Set the function used to broadcast a single `int` value via MPI.  The
    default is `NULL`. */
void set_bm_broadcast_func(bm *self,
                           void (*broadcast)(void *ctx, int *),
                           void *broadcast_ctx);

/** Set the minimum duration required for the inner loop.  The default is 1.0
    second. */
void set_bm_preferred_time(bm *, double preferred_time);

/** Set the number of additional outer loop iterations to perform at the
    beginning.  These iterations are used to warm up the benchmark action and
    are therefore ignored. */
void set_bm_num_warmups(bm *, size_t num_warmups);

/** Set the number of repeats in the outer loop.  The default is 2. */
void set_bm_num_repeats(bm *, size_t num_repeats);

/** Set the initial number of repeats in the inner loop.  The default is 1. */
void set_bm_num_subrepeats(bm *, size_t num_subrepeats);

/** Get the current number of repeats in the inner loop. */
size_t get_bm_num_subrepeats(const bm *);

/** Perform a benchmark.  The function is intended to be used in the following
    manner:

        while (with_bm(...)) {
            // code to be benchmarked goes here
        }

*/
int with_bm(bm *);

/** Print the statistics of the benchmark. */
void print_bm_stats(const bm *, const char *prefix);

#ifdef __cplusplus
}
#endif
#endif
