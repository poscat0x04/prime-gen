#include "bn.h"
#include <alloca.h>
#include <string.h>

bool BN_sqr(BIGINT *r, const BIGINT *a)
{
  bool ret = bn_sqr_fixed_top(r, a);
  bn_correct_top(r);
  return ret;
}

bool bn_sqr_fixed_top(BIGINT *r, const BIGINT *a)
{
  bool ret = false;
  BIGINT *tmp, *rr;

  int al = a->top;
  if (al <= 0) {
    r->top = 0;
    r->neg = 0;
    return true;
  }

  if (a == r) {
    BN_alloca(rr)
  } else {
    rr = r;
  }
  BN_alloca(tmp)
  if (rr == NULL || tmp == NULL)
    goto err;

  int max = 2 * al;               /* Non-zero (from above) */
  if (bn_wexpand(rr, max) == NULL)
    goto err;

  if (al == 4) {
    u64 t[8];
    bn_sqr_normal(rr->d, a->d, 4, t);
  } else if (al == 8) {
    u64 t[16];
    bn_sqr_normal(rr->d, a->d, 8, t);
  } else {
    if (bn_wexpand(tmp, max) == NULL)
      goto err;
    bn_sqr_normal(rr->d, a->d, al, tmp->d);
  }

  rr->neg = 0;
  rr->top = max;
  if (r != rr && BN_copy(r, rr) == NULL)
    goto err;

  ret = true;
err:
  if (a == r)
    BN_free_alloca(rr);
  BN_free_alloca(tmp);
  return ret;
}

void bn_sqr_normal(u64 *r, const u64 *a, int n, u64 *tmp)
{
  int i, j, max;
  const u64 *ap;
  u64 *rp;

  max = n * 2;
  ap = a;
  rp = r;
  rp[0] = rp[max - 1] = 0;
  rp++;
  j = n;

  if (--j > 0) {
    ap++;
    rp[j] = bn_mul_words(rp, ap, j, ap[-1]);
    rp += 2;
  }

  for (i = n - 2; i > 0; i--) {
    j--;
    ap++;
    rp[j] = bn_mul_add_words(rp, ap, j, ap[-1]);
    rp += 2;
  }

  bn_add_words(r, r, r, max);
  bn_sqr_words(tmp, a, n);
  bn_add_words(r, r, tmp, max);
}
