#include <math.h>
#include "math.h"
#include "statistics.h"

const struct statistics_state statistics_initial = {0, 0, 0, INFINITY};

void statistics_update(struct statistics_state *self, double x)
{
    /* Welford's algorithm for updating mean and standard deviation */
    double mean = self->mean;
    const double delta = x - mean;
    const size_t n = ++self->n;
    mean += delta / n;
    self->mean = mean;
    self->m2 += delta * (x - mean);
    self->min = min_d(self->min, x);
}

struct statistics statistics_get(const struct statistics_state *self)
{
    struct statistics out;
    const size_t n = self->n;
    out.mean = n < 1 ? NAN : self->mean;
    out.stdev = n < 2 ? NAN : sqrt(self->m2 / (n - 1));
    out.min = self->min;
    return out;
}
