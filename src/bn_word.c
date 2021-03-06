#include "bn.h"
#include <assert.h>
#include <stdlib.h>

/// Adds a u64 number to a bigint inplace
/// \param a The bigint
/// \param w The u64 number
/// \return Whether the operation is successful
bool BN_add_word(BIGINT *a, u64 w) {
  u64 l;
  int i;

  if (!w)
    return true;
  if (BN_is_zero(a))
    return BN_set_word(a, w);

  if (a->neg) {
    a->neg = 0;
    bool r = BN_sub_word(a, w);
    if (!BN_is_zero(a))
      a->neg = !(a->neg);
    return r;
  }
  for (i = 0; w != 0 && i < a->top; i++) {
    a->d[i] = l = (a->d[i] + w) ;
    w = (w > l) ? 1 : 0;
  }
  // overflow
  if (w && i == a->top) {
    if (bn_wexpand(a, a->top + 1) == NULL)
      return false;
    a->top++;
    a->d[i] = w;
  }
  return true;
}

/// Subtracts a u64 number from a bigint inplace
/// \param a The bigint
/// \param w The u64 number
/// \return Whether the operation is successful
bool BN_sub_word(BIGINT *a, u64 w) {
  int i;

  if (!w)
    return true;
  if (BN_is_zero(a)) {
    bool r = BN_set_word(a, w);
    if (r)
      BN_set_negative(a, true);
    return r;
  }

  /* handle 'a' when negative */
  if (a->neg) {
    a->neg = 0;
    bool r = BN_add_word(a, w);
    a->neg = 1;
    return r;
  }

  if ((a->top == 1) && (a->d[0] < w)) {
    a->d[0] = w - a->d[0];
    a->neg = 1;
    return true;
  }
  i = 0;
  while (true) {
    if (a->d[i] >= w) {
      a->d[i] -= w;
      break;
    } else {
      a->d[i] = (a->d[i] - w) ;
      i++;
      w = 1;
    }
  }
  if ((a->d[i] == 0) && (i == (a->top - 1)))
    a->top--;
  return true;
}

/// Multiplies a u64 number to a bigint inplace
/// \param a The bigint
/// \param w The u64 number
/// \return Whether the operation is successful
bool BN_mul_word(BIGINT *a, u64 w) {
  u64 ll;

  if (a->top) {
    if (w == 0)
      BN_zero(a);
    else {
      ll = bn_mul_words(a->d, a->d, a->top, w);
      if (ll) {
        if (bn_wexpand(a, a->top + 1) == NULL)
          return 0;
        a->d[a->top++] = ll;
      }
    }
  }
  return true;
}

/// Computes ap * w and place result in rp
/// \param rp The array to hold the result
/// \param ap The input bigint, represented as an array
/// \param num The length of <code>ap</code>
/// \param w The other input
/// \return the carry
u64 bn_mul_words(u64 *rp, const u64 *ap, int num, u64 w) {
  u64 c1 = 0;

  if (num <= 0)
    return c1;

  while (num & ~3) {
    mul(rp[0], ap[0], w, c1)
    mul(rp[1], ap[1], w, c1)
    mul(rp[2], ap[2], w, c1)
    mul(rp[3], ap[3], w, c1)
    ap += 4;
    rp += 4;
    num -= 4;
  }
  if (num) {
    mul(rp[0], ap[0], w, c1)
    if (--num == 0)
      return c1;
    mul(rp[1], ap[1], w, c1)
    if (--num == 0)
      return c1;
    mul(rp[2], ap[2], w, c1)
  }
  return c1;
}

/// Divides a with w inplace
/// \param a The BIGINT dividend
/// \param w The divisor
/// \return The remainder
u64 BN_div_word(BIGINT *a, u64 w) {
  u64 ret = 0;
  int i, j;

  if (!w)
    /* actually this an error (division by zero) */
    return (u64)-1;
  if (a->top == 0)
    return 0;

  /* normalize input (align the highest significant bit)
   * (so bn_div_words doesn't complain) */
  j = BN_BITS2 - BN_num_bits_word(w);
  w <<= j;
  if (!BN_lshift(a, a, j))
    return (u64)-1;

  for (i = a->top - 1; i >= 0; i--) {
    u64 l, d;

    l = a->d[i];
    d = bn_div_words(ret, l, w);
    ret = (l - ((d * w) )) ;
    a->d[i] = d;
  }
  if ((a->top > 0) && (a->d[a->top - 1] == 0))
    a->top--;
  ret >>= j;
  if (!a->top)
    a->neg = 0; /* don't allow negative zero */
  return ret;
}

#ifdef ASM
u64 bn_div_words(u64 h, u64 l, u64 d) {
  u64 ret, waste;
  asm( "divq      %4"
     : "=a"(ret), "=d"(waste)
     : "a"(l), "d"(h), "r"(d)
     : "cc");

  return ret;
}
#else
/// Divides a 128 bit integer with a 64 bit integer, the remainder is not kept
/// \param h The higher bit of the 128 bit integer (base 2^64)
/// \param l The lower bit of the 128 bit integer (base 2^64)
/// \param d The divisor
/// \return The quotient
u64 bn_div_words(u64 h, u64 l, u64 d) {
  u64 dh, dl, q, ret = 0, th, tl, t;
  int i, count = 2;

  if (d == 0)
    return UINT64_MAX;

  i = BN_num_bits_word(d);
  assert((i == BN_BITS2) || (h <= (u64)1 << i));

  i = BN_BITS2 - i;
  if (h >= d)
    h -= d;

  if (i) {
    d <<= i;
    h = (h << i) | (l >> (BN_BITS2 - i));
    l <<= i;
  }
  dh = (d & BN_MASK2h) >> BN_BITS4;
  dl = (d & BN_MASK2l);
  while (true) {
    if ((h >> BN_BITS4) == dh)
      q = BN_MASK2l;
    else
      q = h / dh;

    th = q * dh;
    tl = dl * q;
    while (true) {
      t = h - th;
      if ((t & BN_MASK2h) ||
          ((tl) <= ((t << BN_BITS4) | ((l & BN_MASK2h) >> BN_BITS4))))
        break;
      q--;
      th -= dh;
      tl -= dl;
    }
    t = (tl >> BN_BITS4);
    tl = (tl << BN_BITS4) & BN_MASK2h;
    th += t;

    if (l < tl)
      th++;
    l -= tl;
    if (h < th) {
      h += d;
      q--;
    }
    h -= th;

    if (--count == 0)
      break;

    ret = q << BN_BITS4;
    h = ((h << BN_BITS4) | (l >> BN_BITS4)) ;
    l = (l & BN_MASK2l) << BN_BITS4;
  }
  ret |= q;
  return ret;
}
#endif

/// Computes a - b and places the result in r
/// \param r An array to hold the result
/// \param a The bigint input
/// \param b The other bigint input
/// \param n The length of the two inputs
/// \return The carry
#ifdef ASM
u64 bn_sub_words(u64 *rp, const u64 *ap, const u64 *bp,
                      int n)
{
  u64 ret;
  size_t i = 0;

  if (n <= 0)
    return 0;

  asm volatile ("       subq    %0,%0           \n" /* clear borrow */
                "       jmp     1f              \n"
                ".p2align 4                     \n"
                "1:     movq    (%4,%2,8),%0    \n"
                "       sbbq    (%5,%2,8),%0    \n"
                "       movq    %0,(%3,%2,8)    \n"
                "       lea     1(%2),%2        \n"
                "       dec     %1              \n"
                "       jnz     1b              \n"
                "       sbbq    %0,%0           \n"
  :"=&r" (ret), "+c"(n), "+r"(i)
  :"r"(rp), "r"(ap), "r"(bp)
  :"cc", "memory");

  return ret & 1;
}
#else
u64 bn_sub_words(u64 *r, const u64 *a, const u64 *b, int n) {
  u64 t1, t2;
  int carry = 0;

  if (n <= 0)
    return (u64)0;

  while (n) {
    t1 = a[0];
    t2 = b[0];
    r[0] = t1 - t2 - carry;
    if (t1 != t2)
      carry = (t1 < t2);
    a++;
    b++;
    r++;
    n--;
  }
  return carry;
}
#endif

/// Adds bigint array a and b and places the results in c
/// \param r An array to hold the sum
/// \param a The bigint input
/// \param b The other bigint input
/// \param n The length of the two inputs
/// \return The carry
u64 bn_add_words(u64 *r, const u64 *a, const u64 *b, int n) {
  u64 carry = 0, l, t;

  assert(n >= 0);
  if (n <= 0)
    return (u64)0;

  while (n) {
    t = a[0];
    t = (t + carry) ;
    carry = (t < carry);
    l = (t + b[0]) ;
    carry += (l < t);
    r[0] = l;
    a++;
    b++;
    r++;
    n--;
  }
  return (u64)carry;
}

void bn_sqr_words(u64 *r, const u64 *a, int n)
{
  if (n <= 0)
    return;

  while (n & ~3) {
    sqr(r[0], r[1], a[0]);
    sqr(r[2], r[3], a[1]);
    sqr(r[4], r[5], a[2]);
    sqr(r[6], r[7], a[3]);
    a += 4;
    r += 8;
    n -= 4;
  }
  if (n) {
    sqr(r[0], r[1], a[0]);
    if (--n == 0)
      return;
    sqr(r[2], r[3], a[1]);
    if (--n == 0)
      return;
    sqr(r[4], r[5], a[2]);
  }
}
