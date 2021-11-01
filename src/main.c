#include "bn.h"
#include <stdio.h>
#include <alloca.h>
#include <string.h>

int main(void) {
  char num[100];
  scanf("%s", num);
  BIGNUM *bn;
  BN_alloca(bn)
  BIGNUM **bn_ptr = &bn;
  int b = BN_dec2bn(bn_ptr, num);
  printf("%d\n", b);
  BIGNUM *rm;
  BN_alloca(rm)
  BN_zero(rm);
  BIGNUM *divisor;
  BN_alloca(divisor)
  BN_dec2bn(&divisor, "10");
  BN_div(NULL, rm, bn, divisor);
  BN_print(rm);
  BN_free_alloca(bn);
  return 0;
}
