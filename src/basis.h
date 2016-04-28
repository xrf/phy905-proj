#ifndef G_QWP7RA7F2BJR3I8ZWLV4MOXG94MUZ
#define G_QWP7RA7F2BJR3I8ZWLV4MOXG94MUZ
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

struct basis_params {
    unsigned num_shells;
    unsigned num_filled;
};

typedef struct {
    int32_t _ml, _ms;
} h2s_channel;

static inline
h2s_channel make_h2s_channel(int ml, int ms)
{
    h2s_channel l = {ml, ms};
    return l;
}

static inline
int get_h2s_channel_ml(h2s_channel l)
{
    return l._ml;
}

static inline
int get_h2s_channel_ms(h2s_channel l)
{
    return l._ms;
}

/* FIXME: unportable */
static inline
uint64_t pack_h2s_channel(h2s_channel l)
{
    union {
        h2s_channel unpacked;
        uint64_t packed;
    } u;
    u.unpacked = l;
    return u.packed;
}

/* FIXME: unportable */
static inline
h2s_channel unpack_h2s_channel(uint64_t l)
{
    union {
        h2s_channel unpacked;
        uint64_t packed;
    } u;
    u.packed = l;
    return u.unpacked;
}

static const h2s_channel h2s_channel_zero = {0, 0};

static inline
h2s_channel add_h2s_channel(h2s_channel l1, h2s_channel l2)
{
    return make_h2s_channel(
        get_h2s_channel_ml(l1) + get_h2s_channel_ml(l2),
        get_h2s_channel_ms(l1) + get_h2s_channel_ms(l2)
    );
}

static inline
h2s_channel sub_h2s_channel(h2s_channel l1, h2s_channel l2)
{
    return make_h2s_channel(
        get_h2s_channel_ml(l1) - get_h2s_channel_ml(l2),
        get_h2s_channel_ms(l1) - get_h2s_channel_ms(l2)
    );
}

typedef struct basis_orbital_cache basis_orbital_cache;

#ifdef __cplusplus
}
#endif
#endif
