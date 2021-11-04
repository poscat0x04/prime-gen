#include "prime.h"
#include <alloca.h>
#include <assert.h>
#include <string.h>

bool BN_is_prime(const BIGINT *n, int iterations, bool *is_prime) {
  assert(BN_cmp_word(n, 1) > 0);
  if (BN_cmp_word(n, 2) == 0 || BN_cmp_word(n, 3) == 0) {
    *is_prime = true;
    return true;
  }
  // n >= 4
  if (!BN_is_odd(n)) {
    *is_prime = false;
    return true;
  }

  BN_init(r)
  BN_init(d)
  BN_init(n_minus_one)
  BN_init(a)
  BN_init(j)
  MONT_PARAMS *mont_params;
  RNG_PARAMS *rng_params;

  if (!BN_zero(r))
    goto err;
  if (BN_copy(d, n) == NULL
      || BN_copy(n_minus_one, n) == NULL
      || !BN_sub_word(n_minus_one, 1)
      || !BN_sub_word(d, 2)
      || ((rng_params = to_rng_params(d, false)) == NULL)
      || ((mont_params = init_mont(n)) == NULL)
      || !BN_add_word(d, 1))
    goto err;
  do {
    if (!BN_rshift(d, d, 1)
        || !BN_add_word(r, 1))
      goto err;
  } while (!BN_is_odd(d));
  for (int i = 1; i <= iterations; i++) {
    if (!bn_gen_pred(a, rng_params, bn_bigger_than_one)
        || !BN_mod_exp_mont(a, a, d, mont_params))
      goto err;
    if (BN_eq(a, n_minus_one) || BN_cmp_word(a, 1) == 0)
      continue;
    if (!BN_zero(j))
      goto err;
    for (; BN_ucmp(j, r) < 0; BN_add_word(j, 1)) {
      BN_mod_sqr(a, a, n);
      if (BN_eq(a, n_minus_one))
        goto end;
    }
    *is_prime = false;
    return true;
end:;
  }
  *is_prime = true;
  return true;
err:
  BN_free_allocas(4, r, d, a, n_minus_one);
  return false;
}

bool bn_bigger_than_one(const BIGINT *a) {
  return BN_cmp_word(a, 1) > 0;
}
