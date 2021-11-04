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

RNG_PARAMS *to_rng_params(const BIGINT *a, bool copy) {
  if (BN_cmp_word(a, 0) <= 0)
    return NULL;
  RNG_PARAMS *params = malloc(sizeof(RNG_PARAMS));
  if (params == NULL)
    return NULL;

  params->top = a->top;
  if (copy) {
    if ((params->d = malloc(params->top * sizeof(*params->d))) == NULL) {
      free(params);
      return NULL;
    }
    memcpy(params->d, a->d, params->top * sizeof(*params->d));
  } else
    params->d = a->d;
  params->hd_used_bits = BN_num_bits_word(a->d[a->top - 1]);
  return params;
}

bool bn_gen(BIGINT *r, const RNG_PARAMS *params) {
  if (bn_expand_nocpy(r, params->top) == NULL)
    return false;

  u64 m;
  int i;
beginning:
  while ((m = next() & ((1 << params->hd_used_bits) - 1)) > params->d[params->top - 1]);
  r->d[params->top - 1] = m;
  // m == highest digit
  for (i = params->top - 2; i >= 0; i--) {
    m = next();
    r->d[i] = m;
  }
  for (i = params->top - 1; i >= 0; i--) {
    if (r->d[i] < params->d[i]) {
      goto out;
    } else if (r->d[i] >= params->d[i]) {
      goto beginning;
    }
  }
out:
  r->neg = false;
  r->top = params->top;
  while(r->top && r->d[r->top - 1] == 0) {
    r->top--;
  }
  return true;
}

bool bn_gen_pred(BIGINT *r, const RNG_PARAMS *params, bool (*pred)(const BIGINT *)) {
  do {
    if (!bn_gen(r, params))
      return false;
  } while (!pred(r));
  return true;
}
