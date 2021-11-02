#include "bn.h"
#include <alloca.h>
#include <assert.h>
#include <string.h>

bool BN_lshift(BIGINT *r, const BIGINT *a, int n) {
  int ret;

  if (n < 0) {
    return false;
  }

  ret = bn_lshift_fixed_top(r, a, n);
  bn_correct_top(r);
  return ret;
}

bool bn_lshift_fixed_top(BIGINT *r, const BIGINT *a, int n) {
  int i, nw;
  unsigned int lb, rb;
  u64 *t, *f;
  u64 l, m, rmask;

  assert(n >= 0);

  nw = n / BN_BITS2;
  if (bn_wexpand(r, a->top + nw + 1) == NULL)
    return 0;

  if (a->top != 0) {
    lb = (unsigned int) n % BN_BITS2;
    rb = BN_BITS2 - lb;
    rb %= BN_BITS2;      /* say no to undefined behaviour */
    rmask = (u64) 0 - rb; /* rmask = 0 - (rb != 0) */
    rmask |= rmask >> 8;
    f = &(a->d[0]);
    t = &(r->d[nw]);
    l = f[a->top - 1];
    t[a->top] = (l >> rb) & rmask;
    for (i = a->top - 1; i > 0; i--) {
      m = l << lb;
      l = f[i - 1];
      t[i] = (m | ((l >> rb) & rmask));
    }
    t[0] = (l << lb);
  } else {
    /* shouldn't happen, but formally required */
    r->d[nw] = 0;
  }
  if (nw != 0)
    memset(r->d, 0, sizeof(*t) * nw);

  r->neg = a->neg;
  r->top = a->top + nw + 1;

  return true;
}

bool BN_rshift(BIGINT *r, const BIGINT *a, int n) {
  bool ret = false;

  if (n < 0) {
    return false;
  }

  ret = bn_rshift_fixed_top(r, a, n);

  bn_correct_top(r);
  return ret;
}

bool bn_rshift_fixed_top(BIGINT *r, const BIGINT *a, int n) {
  int i, top, nw;
  unsigned int lb, rb;
  u64 *t, *f;
  u64 l, m, mask;

  assert(n >= 0);

  nw = n / BN_BITS2;
  if (nw >= a->top) {
    /* shouldn't happen, but formally required */
    BN_zero(r);
    return true;
  }

  rb = (unsigned int) n % BN_BITS2;
  lb = BN_BITS2 - rb;
  lb %= BN_BITS2;            /* say no to undefined behaviour */
  mask = (u64) 0 - lb;   /* mask = 0 - (lb != 0) */
  mask |= mask >> 8;
  top = a->top - nw;
  if (r != a && bn_wexpand(r, top) == NULL)
    return false;

  t = &(r->d[0]);
  f = &(a->d[nw]);
  l = f[0];
  for (i = 0; i < top - 1; i++) {
    m = f[i + 1];
    t[i] = (l >> rb) | ((m << lb) & mask);
    l = m;
  }
  t[i] = l >> rb;

  r->neg = a->neg;
  r->top = top;

  return true;
}

bool BN_rshift_digits(BIGINT *r, const BIGINT *a, int digits) {
  if (digits == 0) {
    return true;
  }

  assert(digits > 0);

  BIGINT *rr;
  bool ret = false;

  if (r == a)
    BN_alloca(rr)
  else
    rr = r;

  int l = a->top - digits;
  if (l <= 0) {
    ret = BN_zero(rr);
  } else {
    bn_wexpand(rr, l);
    rr->top = l;
    u64 *dp = &(a->d[digits]);
    if (memcpy(rr->d, dp, l * sizeof(*dp)) != NULL)
      ret = true;
  }

  if (r == a) {
    if (BN_copy(r, rr) == NULL)
      ret = false;
    BN_free_alloca(rr);
  }
  return ret;
}

bool BN_lshift_digits(BIGINT *r, const BIGINT *a, int digits) {
  if (digits == 0) {
    return true;
  }

  assert(digits > 0);

  BIGINT *rr;
  bool ret = false;
  if (r == a)
    BN_alloca(rr)
  else
    rr = r;

  int l = a->top + digits;
  bn_wexpand(rr, l);
  rr->top = l;
  u64 *dp = rr->d;
  memset(rr->d,0, digits *sizeof(*rr->d));
  dp += digits;
  if (memcpy(dp, a->d, a->top * sizeof(*dp)) != NULL)
    ret = true;

  if (r == a) {
    if (BN_copy(r, rr) == NULL)
      ret = false;
    BN_free_alloca(rr);
  }
  return ret;
}
