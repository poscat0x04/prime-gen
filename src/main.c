#include "bn.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
  char num[100];
  scanf("%s", num);
  BIGNUM *bn = BN_new();
  BN_zero(bn);
  BIGNUM **bn_ptr = &bn;
  int b = BN_dec2bn(bn_ptr, num);
  printf("%d\n", b);
  BN_print(bn);
  return 0;
}
