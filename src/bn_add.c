#include "bn.h"

bool BN_add(BIGINT *r, const BIGINT *a, const BIGINT *b)
{
  int ret, r_neg, cmp_res;

  if (a->neg == b->neg) {
    r_neg = a->neg;
    ret = BN_uadd(r, a, b);
  } else {
    cmp_res = BN_ucmp(a, b);
    if (cmp_res > 0) {
      r_neg = a->neg;
      ret = BN_usub(r, a, b);
    } else if (cmp_res < 0) {
      r_neg = b->neg;
      ret = BN_usub(r, b, a);
    } else {
      r_neg = 0;
      BN_zero(r);
      ret = 1;
    }
  }

  r->neg = r_neg;
  return ret;
}

/* signed sub of b from a. */
bool BN_sub(BIGINT *r, const BIGINT *a, const BIGINT *b)
{
  int ret, r_neg, cmp_res;

  if (a->neg != b->neg) {
    r_neg = a->neg;
    ret = BN_uadd(r, a, b);
  } else {
    cmp_res = BN_ucmp(a, b);
    if (cmp_res > 0) {
      r_neg = a->neg;
      ret = BN_usub(r, a, b);
    } else if (cmp_res < 0) {
      r_neg = !b->neg;
      ret = BN_usub(r, b, a);
    } else {
      r_neg = 0;
      BN_zero(r);
      ret = 1;
    }
  }

  r->neg = r_neg;
  return ret;
}

/* unsigned add of b to a, r can be equal to a or b. */
bool BN_uadd(BIGINT *r, const BIGINT *a, const BIGINT *b)
{
  int max, min, dif;
  const u64 *ap, *bp;
  u64 *rp, carry, t1, t2;

  if (a->top < b->top) {
    const BIGINT *tmp;

    tmp = a;
    a = b;
    b = tmp;
  }
  max = a->top;
  min = b->top;
  dif = max - min;

  if (bn_wexpand(r, max + 1) == NULL)
    return 0;

  r->top = max;

  ap = a->d;
  bp = b->d;
  rp = r->d;

  carry = bn_add_words(rp, ap, bp, min);
  rp += min;
  ap += min;

  while (dif) {
    dif--;
    t1 = *(ap++);
    t2 = (t1 + carry) & BN_MASK2;
    *(rp++) = t2;
    carry &= (t2 == 0);
  }
  *rp = carry;
  r->top += carry;

  r->neg = 0;
  return 1;
}

/* unsigned subtraction of b from a, a must be larger than b. */
bool BN_usub(BIGINT *r, const BIGINT *a, const BIGINT *b)
{
  int max, min, dif;
  u64 t1, t2, borrow, *rp;
  const u64 *ap, *bp;


  max = a->top;
  min = b->top;
  dif = max - min;

  if (dif < 0) {              /* hmm... should not be happening */
    return 0;
  }

  if (bn_wexpand(r, max) == NULL)
    return 0;

  ap = a->d;
  bp = b->d;
  rp = r->d;

  borrow = bn_sub_words(rp, ap, bp, min);
  ap += min;
  rp += min;

  while (dif) {
    dif--;
    t1 = *(ap++);
    t2 = (t1 - borrow) & BN_MASK2;
    *(rp++) = t2;
    borrow &= (t1 == 0);
  }

  while (max && *--rp == 0)
    max--;

  r->top = max;
  r->neg = 0;
  return 1;
}
