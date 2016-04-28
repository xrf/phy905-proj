#ifndef G_OPBQJIFJ4CFS6F2WOPWHILBSDNAA8
#define G_OPBQJIFJ4CFS6F2WOPWHILBSDNAA8
#include "macros.h"
#include "compat/noreturn_begin.h"
#ifdef __cplusplus
extern "C" {
#endif

/** Abort with an error if the expression returns a nonzero value. */
#define xtry(expr)                                                      \
    xtry_(                                                              \
        (expr),                                                         \
        __FILE__ ":" stringify(__LINE__),                               \
        __func__,                                                       \
        #expr)

/** Abort with an error if the expression is zero. */
#define xensure(expr)                                                   \
    xensure_(                                                           \
        !!(expr),                                                       \
        __FILE__ ":" stringify(__LINE__),                               \
        __func__,                                                       \
        #expr)

/** Abort with an error if the expression is null. */
#define xensure_p(expr)                                                 \
    xensure_p_(                                                         \
        (expr),                                                         \
        __FILE__ ":" stringify(__LINE__),                               \
        __func__,                                                       \
        #expr)

#define xmalloc(size) xensure_p(malloc(size))

noreturn
void debug_abort(const char *prefix,
                 const char *func,
                 int err,
                 const char *msg,
                 const char *expr);

static inline
void xtry_(int err,
           const char *prefix,
           const char *func,
           const char *expr)
{
    if (!err)
        return;
    debug_abort(prefix, func, err, 0, expr);
}

static inline
void xensure_(int val,
              const char *prefix,
              const char *func,
              const char *expr)
{
    if (val)
        return;
    debug_abort(prefix, func, 0, "condition not satisfied:", expr);
}

static inline
void *xensure_p_(void *val,
                 const char *prefix,
                 const char *func,
                 const char *expr)
{
    if (val)
        return val;
    debug_abort(prefix, func, 0, "did not expect null:", expr);
}

#ifdef __cplusplus
}
#endif
#include "compat/noreturn_end.h"
#endif
