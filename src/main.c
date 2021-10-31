#include "bn.h"
#include <assert.h>
#include <stdio.h>
#include <alloca.h>
#include <string.h>

int main(void) {
  char num[100];
  scanf("%s", num);
  BIGNUM *bn;
  BN_alloca(bn);
  BIGNUM **bn_ptr = &bn;
  int b = BN_dec2bn(bn_ptr, num);
  printf("%d\n", b);
  BN_print(bn);
  BN_free_alloca(bn);
  return 0;
}
