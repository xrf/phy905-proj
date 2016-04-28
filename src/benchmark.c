#include <assert.h>
#include <stdio.h>
#include "error.h"
#include "statistics.h"
#include "wclock.h"
#include "benchmark.h"

static wclock myclock;

static int myclock_initialized;

void mysecond_init(void)
{
    if (!myclock_initialized) {
        xtry(init_wclock(&myclock));
        myclock_initialized = 1;
    }
}

double mysecond(void)
{
    if (!myclock_initialized) {
        mysecond_init();
    }
    return get_wclock(&myclock);
}

int benchmark_with(size_t *i,
                   double *time,
                   size_t *count,
                   double preferred_time,
                   double (*get_time)(void),
                   void (*broadcast)(void *ctx, int *),
                   void *broadcast_ctx)
{
    int keep_going;
    double timediff = 0, now = 0;
    ++*i;
    if (*i < *count) {
        return 1;
    }
    assert(get_time || broadcast);
    if (get_time) {
        now = (*get_time)();
        timediff = now - *time;
        keep_going = timediff < preferred_time;
    }
    if (broadcast) {
        (*broadcast)(broadcast_ctx, &keep_going);
    }
    if (keep_going) {
        /* if it's too short, double the number of repeats */
        *i = 0;
        *count *= 2;
        if (get_time) {
            *time = now;
        }
        return 1;
    }
    if (get_time) {
        *time = timediff / *count;
    }
    return 0;
}

bm make_bm(void)
{
    bm self;
    self.stats = statistics_initial;
    self.num_repeats = 2;
    self.num_subrepeats = 1;
    self.preferred_time = 1.;
    self.repeat_index = (size_t)(-1);
    self._num_skipped = 0;
    self._get_time = &mysecond;
    self._broadcast = NULL;
    self._broadcast_ctx = NULL;
    mysecond_init();
    return self;
}

void set_bm_time_func(bm *self, double (*get_time)(void))
{
    self->_get_time = get_time;
}

void set_bm_broadcast_func(bm *self,
                           void (*broadcast)(void *ctx, int *),
                           void *broadcast_ctx)
{
    self->_broadcast = broadcast;
    self->_broadcast_ctx = broadcast_ctx;
}

void set_bm_preferred_time(bm *self, double preferred_time)
{
    self->preferred_time = preferred_time;
}

void set_bm_num_warmups(bm *self, size_t num_warmups)
{
    self->num_repeats -= self->_num_skipped;
    self->_num_skipped = num_warmups;
    self->num_repeats += self->_num_skipped;
}

void set_bm_num_repeats(bm *self, size_t num_subrepeats)
{
    self->num_subrepeats = num_subrepeats + self->_num_skipped;
}

void set_bm_num_subrepeats(bm *self, size_t num_subrepeats)
{
    if (num_subrepeats < 1) {
        num_subrepeats = 1;
    }
    self->num_subrepeats = num_subrepeats;
}

size_t get_bm_num_subrepeats(const bm *self)
{
    return self->num_subrepeats;
}

int with_bm(bm *self)
{
    if (self->repeat_index == (size_t)(-1)) {
        goto start;
    }
inner:
    if (benchmark_with(&self->subrepeat_index,
                       &self->time,
                       &self->num_subrepeats,
                       self->preferred_time,
                       self->_get_time,
                       self->_broadcast,
                       self->_broadcast_ctx)) {
        return 1;
    }
    if (self->_get_time && self->repeat_index >= self->_num_skipped) {
        statistics_update(&self->stats, self->time);
    }
start:
    ++self->repeat_index;
    if (self->repeat_index < self->num_repeats) {
        self->subrepeat_index = (size_t)(-1);
        if (self->_get_time) {
            self->time = (*self->_get_time)();
        }
        goto inner;
    }
    return 0;
}

void print_bm_stats(const bm *self, const char *prefix)
{
    if (self->_get_time) {
        struct statistics st = statistics_get(&self->stats);
        printf(("%smin = %.17g\n"
                "%smean = %.17g\n"
                "%sstdev = %.17g\n"
                "%snum_subrepeats = %zu\n"),
               prefix,
               st.min,
               prefix,
               st.mean,
               prefix,
               st.stdev,
               prefix,
               get_bm_num_subrepeats(self));
        fflush(stdout);
    }
}
