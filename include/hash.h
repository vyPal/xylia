#ifndef XYL_HASH_H
#define XYL_HASH_H

#include <stdint.h>
#include <string.h>

#define HASH_TRUE ((int64_t)(0x9e3779b97f4a7c15ull & 0x7fffffffffffffffull))
#define HASH_FALSE ((int64_t)(0xbf58476d1ce4e5b9ull & 0x7fffffffffffffffull))

#define HASH_NIL ((int64_t)(0x94d049bb133111ebull & 0x7fffffffffffffffull))

static inline int64_t to_positive_int64(uint64_t x) {
  return (int64_t)(x & 0x7fffffffffffffffull);
}

static inline int64_t hash_number(int64_t x) {
  uint64_t z = (uint64_t)x + 0x9e3779b97f4a7c15ull;
  z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
  z = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
  z = z ^ (z >> 31);
  return to_positive_int64(z);
}

static inline int64_t hash_float(double f) {
  if (f == 0.0)
    f = 0.0;
  uint64_t bits;
  memcpy(&bits, &f, sizeof(f));
  return hash_number((int64_t)bits);
}

static inline int64_t hash_string(const char *s, size_t len) {
  uint64_t hash = 1469598103934665603ull;
  for (size_t i = 0; i < len; i++) {
    hash ^= (uint8_t)s[i];
    hash *= 1099511628211ull;
  }
  return to_positive_int64(hash);
}

#endif
