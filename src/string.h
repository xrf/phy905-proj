#ifndef G_3X3ZQ87OUQBFL5HCBD69FDWXADZOM
#define G_3X3ZQ87OUQBFL5HCBD69FDWXADZOM
#include <stdarg.h>
#ifdef __cplusplus
extern "C" {
#endif

/** Call `snprintf` on the given buffer, shift the pointer by the length of
    the string written (without the terminating null character), and decrease
    the size by the same amount.  On success, zero is returned.  If the buffer
    is not long enough, the amount of additional buffer space required is
    returned (including the terminating null character).  As with `snprintf`,
    the result is always null-terminated unless the size is zero.  If an error
    occurs, a negative value is returned. */
int siprintf(char **ptr, size_t *size, const char *format, ...);

/** See docs of `siprintf`. */
int vsiprintf(char **ptr, size_t *size,
              const char *format, va_list vlist);

#ifdef __cplusplus
}
#endif
#endif
