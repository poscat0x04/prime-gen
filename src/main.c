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
  printf("%s", BN_bn2dec(bn));
  return 0;
}
