#ifndef XYL_RANDOM_H
#define XYL_RANDOM_H

#include <stdint.h>

uint64_t get_seed(void);
void mt_seed(uint32_t seed);
uint64_t splitmix_u64(uint64_t *x);
void mt_seed_u64(uint64_t seed64);
void mt_twist(void);
uint32_t mt_rand_u32(void);
double mt_rand_double01(void);
int64_t mt_rand_range(int64_t lo, int64_t hi);

#endif
