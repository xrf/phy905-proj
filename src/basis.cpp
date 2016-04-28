#include <stddef.h>
#include <stdint.h>
#include <unordered_map>
#include "basis.h"

struct basis_orbital_cache {
    std::unordered_map<uint64_t, size_t> _chantable1;
    std::unordered_map<uint64_t, size_t> _chantable2;
};
