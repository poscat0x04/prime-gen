#include "prime.h"

bool BN_gen_prime_bits(BIGINT *r, int iterations, int bits) {
  bool is_prime = false;
  RNG_PARAMS *params = bn_gen_bits_params(bits);
  if (params == NULL)
    return false;
  do {
    if (!bn_gen_bits(r, bits, params)
        || !BN_is_prime(r, iterations, &is_prime)) {
      free_rng_params(params);
      return false;
    }
  } while (!is_prime);
  free_rng_params(params);
  return true;
}
