#include "bn.h"
#include <assert.h>
#include <string.h>
#include <stdarg.h>

u64 *bn_expand_internal(BIGINT *b, int words) {
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

BIGINT *bn_expand2(BIGINT *b, int words) {
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

/// Expands the max size of a bignum
/// \param a The bignum to expand
/// \param words The size of the bignum in words
/// \return The reference to the expanded bignum, if successful,
/// NULL otherwise
BIGINT *bn_wexpand(BIGINT *a, int words) {
  return (words <= a->dmax) ? a : bn_expand2(a, words);
}

BIGINT *bn_expand_nocpy(BIGINT *a, int words) {
  if (words > (INT_MAX / (4 * BN_BITS2))) {
    return NULL;
  }
  if (a->d != NULL)
    free(a->d);
  if ((a->d = malloc(words * sizeof(*a->d))) == NULL)
    return NULL;
  a->dmax = words;
  return a;
}

/// Frees a heap allocated bignum
/// \param a The heap allocated bignum to free
void BN_free(BIGINT *a) {
  if (a == NULL)
    return;
  free(a->d);
  free(a);
}

/// Frees a stack allocated bignum
/// \param a The stack allocated bignum to free
void BN_free_alloca(BIGINT *a) {
  if (a == NULL)
    return;
  free(a->d);
}

void BN_free_allocas(int count,...) {
  va_list ap;
  va_start(ap, count);
  for (int i = 0; i < count; i++)
    BN_free_alloca(va_arg(ap, BIGINT *));
}

/// Allocates a bignum on the heap
/// \return The reference to the allocated bignum, if successful,
/// NULL otherwise
BIGINT *BN_new() {
  BIGINT *ret;
  if ((ret = malloc(sizeof(*ret))) == NULL) {
    return NULL;
  }
  memset(ret, 0, sizeof(*ret));
  return ret;
}

/// Duplicates a bigint
/// \param a The bigint to duplicate
/// \return The reference to the duplicated bigint, if successful,
/// NULL otherwise
BIGINT *BN_dup(const BIGINT *a) {
  BIGINT *t;

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

/// Copies a bignum to another bignum
/// \param a The bigint to copy to
/// \param b The bigint to copy from
/// \return A reference to the bigint copied to, if successful,
/// NULL otherwise
BIGINT *BN_copy(BIGINT *a, const BIGINT *b) {
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

inline void BN_clear(BIGINT *a) {
  memset(a, 0, sizeof(*a));
}

BIGINT *BN_move(BIGINT *a, BIGINT *b) {
  if (a == b)
    return a;
  if (a->d != NULL)
    free(a->d);
  a->d = b->d;
  a->neg = b->neg;
  a->top = b->top;
  BN_clear(b);
  return a;
}

/// Sets the value of a bigint to a u64 integer
/// \param a The bigint to set
/// \param w The u64 integer
/// \return Whether the operation is successful
bool BN_set_word(BIGINT *a, u64 w) {
  if (bn_wexpand(a, 1) == NULL)
    return false;
  a->neg = false;
  a->d[0] = w;
  a->top = (w ? 1 : 0);
  return true;
}

/// Sets the sign of a bigint
/// \param a The bigint to modify
/// \param neg Set to negative if true and positive if false
void BN_set_negative(BIGINT *a, bool neg) {
  if (neg && !BN_is_zero(a)) {
    a->neg = true;
  } else {
    a->neg = false;
  }
}

void BN_invert(BIGINT *a) {
  a->neg = !a->neg;
}

/// Sets the dmax of a bigint to the highest significant digit
/// \param a The bigint
void bn_correct_top(BIGINT *a) {
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

/// Checks if a bigint is odd
/// \param a The bigint to check
/// \return
bool BN_is_odd(const BIGINT *a) { return (a->top > 0) & (a->d[0] & 1); }

/// Checks if a bigint is zero
/// \param a The bigint to check
/// \return
bool BN_is_zero(const BIGINT *a) { return a->top == 0; }


/// Checks if a bigint is negative
/// \param a The bigint to check
/// \return
bool BN_is_negative(const BIGINT *a) { return a->neg; }

/// Compares the absolute values of two bigints
/// \param a The first bigint
/// \param b The second bigint
/// \return positive if a > b, negative of a < b, zero if a = b
int BN_ucmp(const BIGINT *a, const BIGINT *b) {
  u64 t1, t2, *ap, *bp;
  int i = a->top - b->top;
  if (i != 0)
    return i;
  ap = a->d;
  bp = b->d;
  for (int j = a->top - 1; j >= 0; j--) {
    t1 = ap[j];
    t2 = bp[j];
    if (t1 > t2) {
      return 1;
    } else if (t2 > t1) {
      return -1;
    }
  }
  return 0;
}

int BN_cmp_word(const BIGINT *a, u64 w) {
  assert(a != NULL);
  if (a->top == 0)
    return -1;
  else if (a->top > 1)
    return 1;
  else if (a->d[0] > w)
    return 1;
  else if (a->d[0] < w)
    return -1;
  else
    return 0;
}

bool BN_eq(const BIGINT *a, const BIGINT *b) {
  if (a->neg != b->neg || a->top != b->top)
   return false;
  for (int i = 0; i < a->top; i++) {
    if (a->d[i] != b->d[i])
      return false;
  }
  return true;
}

/// Counts the number of bits a bigint uses
/// \param a The bigint
/// \return The number of bits the bigint uses
int BN_num_bits(const BIGINT *a) {
  int i = a->top - 1;
  if (BN_is_zero(a))
    return 0;
  return ((i * BN_BITS2) + BN_num_bits_word(a->d[i]));
}

/// The number of bits a word(u64) uses
/// \param l The word
/// \return The number of bits the word uses
int BN_num_bits_word(u64 l) {
  u64 x, mask;
  int bits = (l != 0);

  // bisection
  x = l >> 32;
  mask = (0 - x) ;
  mask = (0 - (mask >> (BN_BITS2 - 1)));
  bits += 32 & mask;
  l ^= (x ^ l) & mask;

  x = l >> 16;
  mask = (0 - x) ;
  mask = (0 - (mask >> (BN_BITS2 - 1)));
  bits += 16 & mask;
  l ^= (x ^ l) & mask;

  x = l >> 8;
  mask = (0 - x) ;
  mask = (0 - (mask >> (BN_BITS2 - 1)));
  bits += 8 & mask;
  l ^= (x ^ l) & mask;

  x = l >> 4;
  mask = (0 - x) ;
  mask = (0 - (mask >> (BN_BITS2 - 1)));
  bits += 4 & mask;
  l ^= (x ^ l) & mask;

  x = l >> 2;
  mask = (0 - x) ;
  mask = (0 - (mask >> (BN_BITS2 - 1)));
  bits += 2 & mask;
  l ^= (x ^ l) & mask;

  x = l >> 1;
  mask = (0 - x) ;
  mask = (0 - (mask >> (BN_BITS2 - 1)));
  bits += 1 & mask;

  return bits;
}
