#if defined _WIN32
# include <windows.h>
#elif defined __MACH__
# include <mach/mach_time.h>
#else
# define _POSIX_C_SOURCE 199309L
# include <time.h>
# include <sys/resource.h>
#endif
#include <math.h>
#include "wclock.h"
#ifndef NAN
# define NAN (0./0.)
#endif
#ifdef __cplusplus
extern "C" {
#endif

int init_wclock(wclock *self)
{
#if defined _WIN32
    LARGE_INTEGER freq;
    if (!QueryPerformanceFrequency(&freq)) {
        return 1;
    }
    self->_data = 1. / freq.QuadPart;
#elif defined __MACH__
    mach_timebase_info_data_t base;
    if (mach_timebase_info(&base)) {
        return 1;
    }
    self->_data = 1e-9 * base.numer / base.denom;
#else
    (void)self;
#endif
    return 0;
}

double get_wclock(const wclock *self)
{
#if defined _WIN32
    LARGE_INTEGER count;
    if (!QueryPerformanceCounter(&count)) {
        return NAN;
    }
    return count.QuadPart * self->_data;
#elif defined __MACH__
    return mach_absolute_time() * self->_data;
#else
    struct timespec t;
    (void)self;
    if (clock_gettime(CLOCK_MONOTONIC, &t)) {
        return NAN;
    }
    return t.tv_sec + t.tv_nsec * 1e-9;
#endif
}

double get_wclock_res(const wclock *self)
{
#if defined _WIN32 || defined __MACH__
    return self->_data;
#else
    struct timespec t;
    (void)self;
    if (clock_getres(CLOCK_MONOTONIC, &t)) {
        return NAN;
    }
    return t.tv_sec + t.tv_nsec * 1e-9;
#endif
}

#ifdef __cplusplus
}
#endif
