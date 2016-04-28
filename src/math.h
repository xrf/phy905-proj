#ifndef G_SB2RUTXWMXN6M5I9FQ5YDU3X41PCS
#define G_SB2RUTXWMXN6M5I9FQ5YDU3X41PCS

static inline
double min_d(double x, double y)
{
    /* need to deal with NaNs properly */
    return x < y ? x : x == x ? y : x;
}

#endif
