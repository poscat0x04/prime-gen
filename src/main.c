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
  MONT_PARAMS *params = init_mont(a);
  assert(params != NULL);
  to_mont(b, b, params);
  BN_set_word(a, 1000000);
  MONT_exp(b, b, a, params);
  from_mont(b, b, params);
  BN_println(b);
  return 0;
}
