#ifndef UTILS_H
#define UTILS_H

#include "top_level.h"

// Export

uint64_t round_up_mult_pow_two(uint64_t orig, uint64_t multiple);
uint64_t get_aligned_size(uint64_t obj_size, uint64_t page_size);

#endif