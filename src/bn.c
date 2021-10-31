#include "bn.h"
#include <stdlib.h>
#include <string.h>

u64 *bn_expand_internal(BIGNUM *b, int words) {
  u64 *a = NULL;
  // TODO: handle the case where i64 is too large
  a = malloc(words * sizeof(*a));
  // TODO: handle the case where we are actually shrinking (words < b->top)
  if (b->top > 0)
    memcpy(a, b->d, sizeof(*a) * b->top);
  return a;
};

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

bool bn_dec2bn(BIGNUM **bn, const char *a) {
  BIGNUM *ret = NULL;
  u64 l = 0;
  bool neg = false;
  i64 i, j;
  i64 num;

  if (a == NULL || *a == '\0')
    return 0;
  if (*a == '-') {
    neg = true;
    a++;
  }
}

void bn_correct_top(BIGNUM *bn) {}

bool bn_is_odd(const BIGNUM *a) { return (a->top > 0) & (a->d[0] & 1); }

bool bn_is_zero(const BIGNUM *a) { return a->top == 0; }
