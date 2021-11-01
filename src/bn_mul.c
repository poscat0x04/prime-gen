#include "bn.h"
#include <alloca.h>
#include <string.h>


bool BN_mul(BIGINT *r, const BIGINT *a, const BIGINT *b)
{
  int ret = bn_mul_fixed_top(r, a, b);
  bn_correct_top(r);
  return ret;
}

u64 bn_mul_add_words(u64 *rp, const u64 *ap, int num, u64 w) {
  u64 carry = 0;

  if (num <= 0)
    return carry;

  while (num & ~3) {
    mul_add(rp[0], ap[0], w, carry)
    mul_add(rp[1], ap[1], w, carry)
    mul_add(rp[2], ap[2], w, carry)
    mul_add(rp[3], ap[3], w, carry)
    ap += 4;
    rp += 4;
    num -= 4;
  }
  if (num) {
    mul_add(rp[0], ap[0], w, carry)
    if (--num == 0)
      return carry;
    mul_add(rp[1], ap[1], w, carry)
    if (--num == 0)
      return carry;
    mul_add(rp[2], ap[2], w, carry)
    return carry;
  }

  return carry;
}

/// Calculates a * b
/// \param r An array to store the result
/// \param a The first bignum
/// \param na The length of a
/// \param b The second bignum
/// \param nb The length of b
void bn_mul_normal(u64 *r, u64 *a, int na, u64 *b, int nb) {
  u64 *rr;

  // ensures a is longer than b
  if (na < nb) {
    int itmp;
    u64 *ltmp;

    itmp = na;
    na = nb;
    nb = itmp;
    ltmp = a;
    a = b;
    b = ltmp;

  }
  rr = &(r[na]);
  if (nb <= 0) {
    (void)bn_mul_words(r, a, na, 0);
    return;
  } else
    rr[0] = bn_mul_words(r, a, na, b[0]);

  while (true) {
    if (--nb <= 0)
      return;
    rr[1] = bn_mul_add_words(&(r[1]), a, na, b[1]);
    if (--nb <= 0)
      return;
    rr[2] = bn_mul_add_words(&(r[2]), a, na, b[2]);
    if (--nb <= 0)
      return;
    rr[3] = bn_mul_add_words(&(r[3]), a, na, b[3]);
    if (--nb <= 0)
      return;
    rr[4] = bn_mul_add_words(&(r[4]), a, na, b[4]);
    rr += 4;
    r += 4;
    b += 4;
  }
}

bool bn_mul_fixed_top(BIGINT *r, const BIGINT *a, const BIGINT *b)
{
  bool ret = false;
  int top, al, bl;
  BIGINT *rr;

  al = a->top;
  bl = b->top;

  if ((al == 0) || (bl == 0)) {
    BN_zero(r);
    return true;
  }
  top = al + bl;

  if ((r == a) || (r == b))
    BN_alloca(rr)
  else
    rr = r;

  if (bn_wexpand(rr, top) == NULL)
    goto err;
  rr->top = top;
  bn_mul_normal(rr->d, a->d, al, b->d, bl);

  rr->neg = a->neg ^ b->neg;
  if (r != rr && BN_copy(r, rr) == NULL)
    goto err;

  ret = 1;
err:
  if ((r == a) || (r == b )) {
    BN_free_alloca(rr);
  }
  return ret;
}
