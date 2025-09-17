#ifndef XYL_HASH_H
#define XYL_HASH_H

#include <stdint.h>
#include <string.h>

#define HASH_TRUE 0x9e3779b97f4a7c15ULL
#define HASH_FALSE 0xbf58476d1ce4e5b9ULL

#define HASH_NIL 0x94d049bb133111ebULL

static inline uint64_t hash_number(int64_t x) {
  uint64_t z = (uint64_t)x + 0x9e3779b97f4a7c15ULL;
  z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
  z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
  return z ^ (z >> 31);
}

uint64_t hash_float(double f) {
  if (f == 0.0)
    f = 0.0;
  uint64_t bits;
  memcpy(&bits, &f, sizeof(f));
  return hash_number((int64_t)bits);
}

uint64_t hash_string(const char *s, size_t len) {
  uint64_t hash = 1469598103934665603ULL;
  for (size_t i = 0; i < len; i++) {
    hash ^= (uint8_t)s[i];
    hash *= 1099511628211ULL;
  }
  return hash;
}

#endif
