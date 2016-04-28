#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include "string.h"

int vsiprintf(char **ptr, size_t *size, const char *format, va_list vlist)
{
    char *const dptr = *ptr;
    const size_t dsize = *size;
    int ret = vsnprintf(dptr, dsize, format, vlist);
    if (ret < 0) {
        /* failed for unknown reasons */
        return ret;
    }
    if ((INT_MAX > (size_t)(-1) && (int)(size_t)ret != ret)
        || (size_t)ret >= dsize) {
        /* output was truncated; return the amount of extra space needed */
        if (dsize) {
            ret -= (int)dsize;
            *ptr = dptr + (dsize - 1);
            *size = 1;
        } else if (ret == INT_MAX) {
            /* avoid overflow (note: reachable only when dsize == 0) */
            return INT_MIN;
        }
        return ret + 1;
    }
    *ptr = dptr + (size_t)ret;
    *size = dsize - (size_t)ret;
    return 0;
}

int siprintf(char **ptr, size_t *size, const char *format, ...)
{
    int ret;
    va_list vlist;
    va_start(vlist, format);
    ret = vsiprintf(ptr, size, format, vlist);
    va_end(vlist);
    return ret;
}
