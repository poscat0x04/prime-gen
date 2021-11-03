/* xoshiro256+ based bigint generation
 */

#include "prime.h"
#include <alloca.h>
#include <string.h>
#include <assert.h>

static inline u64 rotl(const u64 x, int k) {
  return (x << k) | (x >> (64 - k));
}

static u64 s[4];

#ifdef ASM
void seed(void) {
  u64 r;
  // don't check for err cause 4 calls probably won't exhaust entropy
  for (int i = 0; i < 4; i++) {
    asm volatile ("rdrand %0" : "=a" (r));
    s[i] = r;
  }
}
#else
void seed (void) {
  FILE *urandom = fopen("/dev/urandom", "r");
  assert(urandom != NULL);
  assert(fread(&s, sizeof(u64), 4, urandom) == 4);
}
#endif

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

RNG_PARAMS *to_rng_params(const BIGINT *a) {
  if (BN_cmp_word(a, 0) <= 0)
    return NULL;
  RNG_PARAMS *params = malloc(sizeof(RNG_PARAMS));

  params->digits = a->top;
  params->highest_digit = a->d[a->top - 1];
  params->hd_used_bits = BN_num_bits_word(params->highest_digit);
  return params;
}

bool bn_gen(BIGINT *r, const RNG_PARAMS *params) {
  if (bn_expand_nocpy(r, params->digits) == NULL)
    return false;
  for (int i = 0; i < params->digits - 1; i++) {
    r->d[i] = next();
  }
  u64 m;
  while ((m = next() & ((1 << params->hd_used_bits) - 1)) > params->highest_digit);
  r->d[params->digits - 1] = m;
  r->top = params->digits;
  return true;
}

bool bn_gen_pred(BIGINT *r, const RNG_PARAMS *params, bool (*pred)(const BIGINT *)) {
  do {
    if (!bn_gen(r, params))
      return false;
  } while (!pred(r));
  return true;
}
