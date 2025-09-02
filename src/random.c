#include <fcntl.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "random.h"

#define MT_N 624
#define MT_M 397
#define MT_MATRIX_A 0x9908b0dfu
#define MT_UPPER_MASK 0x80000000u
#define MT_LOWER_MASK 0x7fffffffu

static uint32_t mt_state[MT_N];
static int mt_index = MT_N + 1;

uint64_t get_seed(void) {
  uint64_t seed = 0;
  int fd = open("/dev/urandom", O_RDONLY);
  if (fd >= 0) {
    ssize_t r = read(fd, &seed, sizeof(seed));
    close(fd);
    if (r == sizeof(seed))
      return seed;
  }

  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);

  seed ^= (uint64_t)ts.tv_nsec;
  seed ^= (uint64_t)ts.tv_sec;
  seed ^= (uint64_t)getpid();
  seed ^= (uint64_t)&ts;

  return seed;
}

void mt_seed(uint32_t seed) {
  if (seed == 0)
    seed = 5489u;

  for (mt_index = 1; mt_index < MT_N; mt_index++) {
    uint32_t prev = mt_state[mt_index - 1];
    mt_state[mt_index] =
        (1812433253u * (prev ^ (prev >> 30)) + (uint32_t)mt_index);
  }

  mt_index = MT_N;
}

uint64_t splitmix_u64(uint64_t *x) {
  uint64_t z = (*x += 0x9E3779B97F4A7C15ull);
  z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
  z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
  return z ^ (z >> 31);
}

void mt_seed_u64(uint64_t seed64) {
  uint32_t init_key[MT_N];
  uint64_t sm = seed64 ? seed64 : 0x1234ull;
  for (int i = 0; i < MT_N; i++)
    init_key[i] = (uint32_t)splitmix_u64(&sm);

  mt_seed(19650218u);
  int i = 1, j = 0, k = MT_N;
  for (k = MT_N; k; k--) {
    uint32_t x = mt_state[i - 1] ^ (mt_state[i - 1] >> 30);
    mt_state[i] = (mt_state[i] ^ (1664525u * x)) + init_key[j] + (uint32_t)j;
    i++;
    j++;
    if (i >= MT_N) {
      mt_state[0] = mt_state[MT_N - 1];
      i = 1;
    }
    if (j >= MT_N)
      j = 0;
  }
  for (k = MT_N - 1; k; k--) {
    uint32_t x = mt_state[i - 1] ^ (mt_state[i - 1] >> 30);
    mt_state[i] = (mt_state[i] ^ (1566083941u * x)) - (uint32_t)i;
    i++;
    if (i >= MT_N) {
      mt_state[0] = mt_state[MT_N - 1];
      i = 1;
    }
  }
  mt_state[0] = 0x80000000u;
  mt_index = MT_N;
}

void mt_twist(void) {
  static const uint32_t mag[2] = {0u, MT_MATRIX_A};
  int i;
  for (i = 0; i < MT_N - MT_M; i++) {
    uint32_t y =
        (mt_state[i] & MT_UPPER_MASK) | (mt_state[i + 1] & MT_LOWER_MASK);
    mt_state[i] = mt_state[i + MT_M] ^ (y >> 1) ^ mag[y & 1u];
  }
  for (; i < MT_N - 1; i++) {
    uint32_t y =
        (mt_state[i] & MT_UPPER_MASK) | (mt_state[i + 1] & MT_LOWER_MASK);
    mt_state[i] = mt_state[i + (MT_M - MT_N)] ^ (y >> 1) ^ mag[y & 1u];
  }
  uint32_t y =
      (mt_state[MT_N - 1] & MT_UPPER_MASK) | (mt_state[0] & MT_LOWER_MASK);
  mt_state[MT_N - 1] = mt_state[MT_M - 1] ^ (y >> 1) ^ mag[y & 1u];
  mt_index = 0;
}

uint32_t mt_rand_u32(void) {
  if (mt_index >= MT_N) {
    if (mt_index > MT_N)
      mt_seed(5489u);
    mt_twist();
  }
  uint32_t y = mt_state[mt_index++];

  y ^= (y >> 11);
  y ^= (y << 7) & 0x9d2c5680u;
  y ^= (y << 15) & 0xefc60000u;
  y ^= (y >> 18);

  return y;
}

double mt_rand_double01(void) {
  uint64_t a = mt_rand_u32() >> 5;
  uint64_t b = mt_rand_u32() >> 6;
  uint64_t u53 = (a << 26) | b;
  return (double)u53 * (1.0 / 9007199254740992.0);
}

int64_t mt_rand_range(int64_t lo, int64_t hi) {
  if (lo > hi) {
    int64_t t = lo;
    lo = hi;
    hi = t;
  }
  uint64_t span = (uint64_t)(hi - lo + 1);
  if (span == 0)
    return (int64_t)mt_rand_u32();
  uint64_t limit = UINT64_MAX - (UINT64_MAX % span);
  uint64_t r;
  do {
    r = ((uint64_t)mt_rand_u32() << 32) | mt_rand_u32();
  } while (r >= limit);
  return (int64_t)(lo + (r % span));
}
