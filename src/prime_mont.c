#include "prime.h"
#include "alloca.h"
#include "string.h"

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
