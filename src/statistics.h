#ifndef G_6UMLVZVYM9XXNO0TCAF1X9HUS1MNA
#define G_6UMLVZVYM9XXNO0TCAF1X9HUS1MNA
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif

/** Contains the mean, standard deviation, and minimum. */
struct statistics {
    double mean, stdev, min;
};

/** For tracking the state of an incremental statistics calculator. */
struct statistics_state {
    size_t n;
    double mean, m2, min;
};

/** Initial state. */
extern const struct statistics_state statistics_initial;

/** Add another value and update the statistics. */
void statistics_update(struct statistics_state *self, double x);

/** Obtain the statistics from the given state. */
struct statistics statistics_get(const struct statistics_state *self);

#ifdef __cplusplus
}
#endif
#endif
