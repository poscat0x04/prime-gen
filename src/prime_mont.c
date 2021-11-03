#include "prime.h"
#include "alloca.h"
#include "string.h"
#include "assert.h"

MONT_PARAMS *init_mont(const BIGINT *n) {
  MONT_PARAMS *params = malloc(sizeof(MONT_PARAMS));
  BIGINT *R = &params->R;
  BIGINT *RR = &params->RR;
  BIGINT *N = &params->N;
  BIGINT *Ni = &params->Ni;
  MONT *One_mont = &params->One;
  BN_init(gcd)
  BN_clear(R);
  BN_clear(N);
  BN_clear(Ni);
  if (!BN_set_word(R, 1)
      || !BN_lshift_digits(R, R, n->top)
      || BN_copy(N, n) == NULL)
    goto err;
  if (!extgcd(gcd, NULL, Ni, R, N) || gcd->dmax != 1 || gcd->d[0] != 1)
    goto err;
  BN_invert(Ni);
  if (!BN_mod_sqr(RR, R, N))
    goto err;
  if (!to_mont(One_mont, &C_BN_one, params))
    goto err;
  BN_free_alloca(gcd);
  return params;
err:
  BN_free_allocas(5, R, RR, N, Ni, gcd);
  free(params);
  return NULL;
}

void free_mont(MONT_PARAMS *params) {
  BN_free_alloca(&params->R);
  BN_free_alloca(&params->N);
  BN_free_alloca(&params->Ni);
  free(params);
}

bool REDC(BIGINT *r, const BIGINT *t, const MONT_PARAMS *params) {
  assert(!BN_is_negative(t));

  BIGINT *rr;
  if (r == t) {
    BN_alloca(rr)
    BN_clear(rr);
  } else
    rr = r;

  bool ret = false;
  int digits = params->R.top - 1;

  if (!BN_mod_digits(rr, t, digits)
      || !BN_mul(rr, rr, &params->Ni)
      || !BN_mod_digits(rr, rr, digits)
      || !BN_mul(rr, rr, &params->N)
      || !BN_add(rr, rr, t)
      || !BN_rshift_digits(rr, rr, digits))
    goto end;
  if (BN_ucmp(rr, &params->N) >= 0) {
    if (!BN_sub(rr, rr, &params->N))
      goto end;
  }
  ret = true;
end:
  if (ret && r == t) {
    BN_move(r, rr);
  }
  return ret;
}

bool to_mont(MONT *r, const BIGINT *a, const MONT_PARAMS *params) {
  BIGINT *rr;
  bool ret = false;

  if (r == a) {
    BN_alloca(rr)
    BN_clear(rr);
  } else
    rr = r;

  if (!BN_mod(rr, a, &params->N)
      || !BN_mul(rr, rr, &params->RR)
      || !REDC(rr, rr, params))
    goto end;
  if (r == a)
    BN_move(r, rr);
  ret = true;
end:
  if (!ret && r == a)
    BN_free_alloca(rr);
  return ret;
}

bool MONT_mul(MONT *r, const MONT *a, const MONT *b, const MONT_PARAMS *params) {
  MONT *rr;
  bool ret = false;

  if (r == a) {
    BN_alloca(rr)
    BN_clear(rr);
  } else
    rr = r;

  if (!BN_mul(rr, a, b)
      || !REDC(rr, rr, params))
    goto end;

  if (r == a)
    BN_move(r, rr);
  ret = true;
end:
  if (!ret && r == a)
    BN_free_alloca(rr);
  return ret;
}

bool MONT_exp(MONT *r, const MONT *a, const BIGINT *e, const MONT_PARAMS *params) {
  assert(!BN_is_negative(e));

  MONT *rr;
  bool ret = false;

  if (r == a) {
    BN_alloca(rr)
    BN_clear(rr);
  } else
    rr = r;

  BN_init(a_tmp)
  BN_init(e_tmp)
  if (BN_copy(a_tmp, a) == NULL
      || BN_copy(e_tmp, e) == NULL
      || BN_copy(rr, &params->One) == NULL)
    goto end;

  while (!BN_is_zero(e_tmp)) {
    if (BN_is_odd(e_tmp))
      if (!MONT_mul(rr, rr, a_tmp, params))
        goto end;
    if (!MONT_mul(a_tmp, a_tmp, a_tmp, params)
        || !BN_rshift(e_tmp, e_tmp, 1))
      goto end;
  }

  if (r == a)
    BN_move(r, rr);
  ret = true;
end:
  if (!ret && r == a)
    BN_free_alloca(rr);
  BN_free_allocas(2, a_tmp, e_tmp);
  return ret;
}

bool BN_mod_exp(BIGINT *r, const BIGINT *a, const BIGINT *e, const BIGINT *m) {
  MONT *rr;
  bool ret = false;

  if (r == a) {
    BN_alloca(rr)
    BN_clear(rr);
  } else
    rr = r;

  MONT_PARAMS *params = init_mont(m);
  if (!to_mont(rr, a, params)
      || !MONT_exp(rr, rr, e, params)
      || !from_mont(rr, rr, params))
    goto end;
  free_mont(params);

  if (r == a)
    BN_move(r, rr);
  ret = true;
end:
  if (!ret && r == a)
    BN_free_alloca(rr);
  return ret;
}

bool BN_mod_exp_mont(BIGINT *r, const BIGINT *a, const BIGINT *e, const MONT_PARAMS *params) {
  MONT *rr;
  bool ret = false;

  if (r == a) {
    BN_alloca(rr)
    BN_clear(rr);
  } else
    rr = r;

  if (!to_mont(rr, a, params)
      || !MONT_exp(rr, rr, e, params)
      || !from_mont(rr, rr, params))
    goto end;

  if (r == a)
    BN_move(r, rr);
  ret = true;
end:
  if (!ret && r == a)
    BN_free_alloca(rr);
  return ret;
}
