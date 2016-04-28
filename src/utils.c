#include <stdarg.h>
#include <stdio.h>
#include "error.h"
#include "mpi.h"
#include "wclock.h"
#include "utils.h"

static wclock clock;

void init_gwclock(void)
{
    xtry(init_wclock(&clock));
}

double get_gwclock(void)
{
    return get_wclock(&clock);
}

int printf0(const char *format, ...)
{
    int e;
    va_list args;
    if (mpi.rank != 0) {
        return 0;
    }
    va_start(args, format);
    e = vprintf(format, args);
    va_end(args);
    return e;
}
