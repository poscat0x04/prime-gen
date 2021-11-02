#include "prime.h"
#include "alloca.h"
#include "string.h"

MONT_PARAMS *init_mont(const BIGINT *n) {
  MONT_PARAMS* params = malloc(sizeof(MONT_PARAMS));
  BIGINT *R = &params->R;
  BIGINT *N = &params->N;
  BIGINT *Ni = &params->Ni;
  BN_clear(R);
  BN_clear(N);
  BN_clear(Ni);
  BN_set_word(R, 1);
  BN_lshift_digits(R, R, n->top);
  BN_copy(N, n);
  BN_init(gcd)
  if (!extgcd(gcd, NULL, Ni, R, N) || gcd->dmax != 1 || gcd->d[0] != 1)
    return NULL;
  BN_invert(Ni);
  return params;
}

void free_mont(MONT_PARAMS *params) {
  BN_free_alloca(&params->R);
  BN_free_alloca(&params->N);
  BN_free_alloca(&params->Ni);
  free(params);
}
