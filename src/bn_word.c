#include "bn.h"
#include <assert.h>
#include <stdlib.h>

bool BN_add_word(BIGNUM *a, u64 w) {
  u64 l;
  int i;

  w &= BN_MASK2;

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
    a->d[i] = l = (a->d[i] + w) & BN_MASK2;
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

bool BN_sub_word(BIGNUM *a, u64 w) {
  int i;

  w &= BN_MASK2;

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
  for (;;) {
    if (a->d[i] >= w) {
      a->d[i] -= w;
      break;
    } else {
      a->d[i] = (a->d[i] - w) & BN_MASK2;
      i++;
      w = 1;
    }
  }
  if ((a->d[i] == 0) && (i == (a->top - 1)))
    a->top--;
  return true;
}

bool BN_mul_word(BIGNUM *a, u64 w) {
  u64 ll;

  w &= BN_MASK2;
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

u64 bn_mul_words(u64 *rp, const u64 *ap, int num, u64 w) {
  u64 carry = 0;
  u64 bl, bh;

  assert(num >= 0);
  if (num <= 0)
    return (u64)0;

  bl = LBITS(w);
  bh = HBITS(w);

  while (num & ~3) {
    mul(rp[0], ap[0], bl, bh, carry);
    mul(rp[1], ap[1], bl, bh, carry);
    mul(rp[2], ap[2], bl, bh, carry);
    mul(rp[3], ap[3], bl, bh, carry);
    ap += 4;
    rp += 4;
    num -= 4;
  }

  while (num) {
    mul(rp[0], ap[0], bl, bh, carry);
    ap++;
    rp++;
    num--;
  }
  return carry;
}

u64 BN_div_word(BIGNUM *a, u64 w) {
  u64 ret = 0;
  int i, j;

  w &= BN_MASK2;

  if (!w)
    /* actually this an error (division by zero) */
    return (u64)-1;
  if (a->top == 0)
    return 0;

  /* normalize input (so bn_div_words doesn't complain) */
  j = BN_BITS2 - BN_num_bits_word(w);
  w <<= j;
  if (!BN_lshift(a, a, j))
    return (u64)-1;

  for (i = a->top - 1; i >= 0; i--) {
    u64 l, d;

    l = a->d[i];
    d = bn_div_words(ret, l, w);
    ret = (l - ((d * w) & BN_MASK2)) & BN_MASK2;
    a->d[i] = d;
  }
  if ((a->top > 0) && (a->d[a->top - 1] == 0))
    a->top--;
  ret >>= j;
  if (!a->top)
    a->neg = 0; /* don't allow negative zero */
  return ret;
}

u64 bn_div_words(u64 h, u64 l, u64 d) {
  u64 dh, dl, q, ret = 0, th, tl, t;
  int i, count = 2;

  if (d == 0)
    return BN_MASK2;

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
  for (;;) {
    if ((h >> BN_BITS4) == dh)
      q = BN_MASK2l;
    else
      q = h / dh;

    th = q * dh;
    tl = dl * q;
    for (;;) {
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
    h = ((h << BN_BITS4) | (l >> BN_BITS4)) & BN_MASK2;
    l = (l & BN_MASK2l) << BN_BITS4;
  }
  ret |= q;
  return ret;
}
