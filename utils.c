#include "utils.h"

uint64_t round_up_mult_pow_two(uint64_t orig, uint64_t multiple){
	return (orig + multiple - 1) & (~(multiple - 1));
}

uint64_t get_aligned_size(uint64_t obj_size, uint64_t page_size){
	int remainder = obj_size % page_size;
    if (remainder == 0)
        return obj_size;

    return obj_size + page_size - remainder;
}