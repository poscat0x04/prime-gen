#include "prime.h"
#include <alloca.h>
#include <string.h>
#include <assert.h>

int main(void) {
  BN_init(a)
  BN_init(b)
  BN_set_word(a, 89);
  BN_set_word(b, 10);
  BN_init(gcd)
  BN_init(x)
  BN_init(y)
  extgcd(gcd, x, y, a, b);
  BN_println(gcd);
  BN_println(x);
  BN_println(y);
  BN_set_word(a, 99);
  BN_set_word(y, 65536);
  BN_mod_exp(b, b, y, a);
  BN_println(b);
  seed();
  BN_lshift_digits(y, y, 1);
  RNG_PARAMS *params = to_rng_params(y, false);
  BN_init(tmp)
  bn_gen(tmp, params);
  BN_println(tmp);
  return 0;
}
