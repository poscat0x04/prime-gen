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
  assert(params->hd_used_bits <= 64);
  if (bn_expand_nocpy(r, params->top) == NULL)
    return false;

  u64 m, mod;
  int i;
  mod = UINT64_C(1) << params->hd_used_bits;
beginning:
  if (params->hd_used_bits == 64)
    while ((m = next()) > params->d[params->top - 1]);
  else
    while ((m = next() & (mod - 1)) > params->d[params->top - 1]);
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

bool bn_fill_bits(BIGINT *r, int bits) {
  int w = (bits + 63) / 64;
  r->top = w;
  if (bn_expand_nocpy(r, w) == NULL)
    return false;
  u64 mod = bits % 64;
  if (mod) {
    for (int i = 0; i < w - 1; i++) {
      r->d[i] = UINT64_MAX;
    }
    mod = UINT64_C(1) << mod;
    r->d[w - 1] = UINT64_MAX % mod;
  } else {
    for (int i = 0; i < w; i++) {
      r->d[i] = UINT64_MAX;
    }
  }
  return true;
}

RNG_PARAMS* bn_gen_bits_params(int bits) {
  if (bits <= 0)
    return NULL;
  BN_init(tmp)
  bn_fill_bits(tmp, bits);
  return to_rng_params(tmp, true);
}

/// Generate a random bigint with exactly n bits
/// \param r The bigint to receive the result
/// \param bits The number of bits in the bigint
/// \param params The rng param given by bn_gen_bits_params
/// \return Whether the operation nis successful
bool bn_gen_bits(BIGINT *r, int bits, RNG_PARAMS *params) {
  int w = (bits + 63) / 64;
  bits = bits % 64;
  if (!bn_gen(r, params)
      || bn_wexpand(r, w) == NULL)
    return false;
  u64 mask = UINT64_C(1) << (bits - 1);
  r->d[r->top - 1] |= mask;
  return true;
}
