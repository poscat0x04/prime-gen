#include "bn.h"
#include <assert.h>
#include <string.h>

u64 *bn_expand_internal(BIGNUM *b, int words) {
  u64 *a = NULL;
  if (words > (INT_MAX / (4 * BN_BITS2))) {
    return NULL;
  }
  a = malloc(words * sizeof(*a));
  assert(b->top <= words);
  if (b->top > 0)
    memcpy(a, b->d, sizeof(*a) * b->top);
  return a;
}

BIGNUM *bn_expand2(BIGNUM *b, int words) {
  if (words > b->dmax) {
    u64 *a = bn_expand_internal(b, words);
    if (!a)
      return NULL;
    if (b->d != NULL)
      free(b->d);
    b->d = a;
    b->dmax = words;
  }
  return b;
}

BIGNUM *bn_wexpand(BIGNUM *a, int words) {
  return (words <= a->dmax) ? a : bn_expand2(a, words);
}

void BN_free(BIGNUM *a) {
  if (a == NULL)
    return;
  free(a->d);
  free(a);
}

void BN_free_alloca(BIGNUM *a) {
  if (a == NULL)
    return;
  free(a->d);
}

BIGNUM *BN_new() {
  BIGNUM *ret;
  if ((ret = malloc(sizeof(*ret))) == NULL) {
    return NULL;
  }
  memset(ret, 0, sizeof(*ret));
  return ret;
}

BIGNUM *BN_dup(const BIGNUM *a) {
  BIGNUM *t;

  if (a == NULL)
    return NULL;

  t = BN_new();
  if (t == NULL)
    return NULL;
  if (!BN_copy(t, a)) {
    BN_free(t);
    return NULL;
  }
  return t;
}

BIGNUM *BN_copy(BIGNUM *a, const BIGNUM *b) {
  int bn_words = b->top;

  if (a == b)
    return a;
  if (bn_wexpand(a, bn_words) == NULL)
    return NULL;

  if (b->top > 0)
    memcpy(a->d, b->d, sizeof(b->d[0]) * bn_words);

  a->neg = b->neg;
  a->top = b->top;
  return a;
}

bool BN_set_word(BIGNUM *a, u64 w) {
  if (bn_wexpand(a, 1) == NULL)
    return false;
  a->neg = false;
  a->d[0] = w;
  a->top = (w ? 1 : 0);
  return true;
}

void BN_set_negative(BIGNUM *a, bool neg) {
  if (neg && !BN_is_zero(a)) {
    a->neg = true;
  } else {
    a->neg = false;
  }
}

void bn_correct_top(BIGNUM *a) {
  u64 *ftl;
  int tmp_top = a->top;

  if (tmp_top > 0) {
    for (ftl = &(a->d[tmp_top]); tmp_top > 0; tmp_top--) {
      ftl--;
      if (*ftl != 0)
        break;
    }
    a->top = tmp_top;
  }
  if (a->top == 0)
    a->neg = false;
}

bool BN_is_odd(const BIGNUM *a) { return (a->top > 0) & (a->d[0] & 1); }

bool BN_is_zero(const BIGNUM *a) { return a->top == 0; }

bool BN_is_negative(const BIGNUM *a) { return a->neg; }

int BN_num_bits(const BIGNUM *a) {
  int i = a->top - 1;
  if (BN_is_zero(a))
    return 0;
  return ((i * BN_BITS2) + BN_num_bits_word(a->d[i]));
}

int BN_num_bits_word(u64 l) {
  u64 x, mask;
  int bits = (l != 0);

  x = l >> 32;
  mask = (0 - x) & BN_MASK2;
  mask = (0 - (mask >> (BN_BITS2 - 1)));
  bits += 32 & mask;
  l ^= (x ^ l) & mask;

  x = l >> 16;
  mask = (0 - x) & BN_MASK2;
  mask = (0 - (mask >> (BN_BITS2 - 1)));
  bits += 16 & mask;
  l ^= (x ^ l) & mask;

  x = l >> 8;
  mask = (0 - x) & BN_MASK2;
  mask = (0 - (mask >> (BN_BITS2 - 1)));
  bits += 8 & mask;
  l ^= (x ^ l) & mask;

  x = l >> 4;
  mask = (0 - x) & BN_MASK2;
  mask = (0 - (mask >> (BN_BITS2 - 1)));
  bits += 4 & mask;
  l ^= (x ^ l) & mask;

  x = l >> 2;
  mask = (0 - x) & BN_MASK2;
  mask = (0 - (mask >> (BN_BITS2 - 1)));
  bits += 2 & mask;
  l ^= (x ^ l) & mask;

  x = l >> 1;
  mask = (0 - x) & BN_MASK2;
  mask = (0 - (mask >> (BN_BITS2 - 1)));
  bits += 1 & mask;

  return bits;
}
