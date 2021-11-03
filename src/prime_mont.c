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
  BN_init(gcd)
  BN_clear(R);
  BN_clear(N);
  BN_clear(Ni);
  if(!BN_set_word(R, 1)
     || !BN_lshift_digits(R, R, n->top)
     || BN_copy(N, n) == NULL)
    goto err;
  if (!extgcd(gcd, NULL, Ni, R, N) || gcd->dmax != 1 || gcd->d[0] != 1)
    goto err;
  BN_invert(Ni);
  if(!BN_mod_sqr(RR, R, N))
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

bool REDC(BIGINT *r, const BIGINT *t, MONT_PARAMS *params) {
  assert(!BN_is_negative(t));

  BIGINT *rr;
  if (r == t)
    BN_alloca(rr)
  else
    rr = r;

  bool ret = false;
  int digits = params->R.top;

  if(!BN_mod_digits(rr, t, digits)
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
