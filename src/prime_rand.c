/* xoshiro256+ based bigint generation
 */

#include "prime.h"

static inline u64 rotl(const u64 x, int k) {
  return (x << k) | (x >> (64 - k));
}

static u64 s[4];

void seed(void) {
  u64 r;
  for (int i = 0; i < 4; i++) {
    asm volatile ("rdrand %0" : "=a" (r));
    s[i] = r;
  }
}

u64 next(void) {
  const u64 result = rotl(s[0] + s[3], 23) + s[0];

  const u64 t = s[1] << 17;

  s[2] ^= s[0];
  s[3] ^= s[1];
  s[1] ^= s[2];
  s[0] ^= s[3];

  s[2] ^= t;

  s[3] = rotl(s[3], 45);

  return result;
}
