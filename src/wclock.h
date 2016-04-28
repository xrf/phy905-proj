#ifndef G_5FP4P4EX8CFF5QNJGX7WJ5L9QH6QS
#define G_5FP4P4EX8CFF5QNJGX7WJ5L9QH6QS
#ifdef __cplusplus
extern "C" {
#endif
/** @file

    Functions for accessing a monotonic wall clock.
*/

typedef struct { double _data; } wclock;

/** Initialize a value needed to access the monotonic wall clock.  The clock
    does not need to be deinitialized.

    @param clock
    An existing `wclock` value to be initialized.

    @return
    Zero on success, nonzero on failure.
*/
int init_wclock(wclock *clock);

/** Retrieve the time from a monotonic wall clock in seconds.

    @param clock
    An `wclock` value previously initialized by `init_wclock`.

    @return
    Duration relative to some unspecified reference time in seconds.
    If an error occurs, `NAN` is returned.

    Due to the use of double-precision floating point numbers, the precision
    is at worst (for an ideal system with hundreds of years in uptime) limited
    to about a few microseconds.  In practice, it is usually much less.
*/
double get_wclock(const wclock *wclock);

/** Obtain the resolution of the monotonic wall clock in seconds.

    @param clock
    An `wclock` value previously initialized by `init_wclock`.

    @return
    Duration relative to some unspecified reference time in seconds.
    If an error occurs, `NAN` is returned.
*/
double get_wclock_res(const wclock *wclock);

#ifdef __cplusplus
}
#endif
#endif
