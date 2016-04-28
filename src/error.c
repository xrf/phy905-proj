#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include "error.h"

void debug_abort(const char *prefix,
                 const char *func,
                 int err,
                 const char *msg,
                 const char *expr)
{
    char buf[512];
    char *p = buf;
    size_t n = sizeof(buf) / sizeof(*buf);

    if (!prefix) {
        prefix = "<??" "?>";            /* avoid trigraphs */
    }
    if (!func) {
        func = "<??" "?>";              /* avoid trigraphs */
    }

    /* store it into a buffer before printing to reduce the risk of printing a
       partial message (e.g. when multiple threads are interleaved) */
    siprintf(&p, &n, "%s:%s: [error", prefix, func);
    if (err) {
        siprintf(&p, &n, " %i", err);
    }
    siprintf(&p, &n, "]");
    if (msg) {
        siprintf(&p, &n, " %s", msg);
    }
    if (expr) {
        siprintf(&p, &n, " %s", expr);
    }

    fprintf(stderr, "%s\n", buf);
    fflush(stderr);
    abort();
}
