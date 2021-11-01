#include "bn.h"
#include <stdio.h>
#include <alloca.h>
#include <string.h>

int main(void) {
  BIGINT *a;
  BN_alloca(a)
  BIGINT *b;
  BN_alloca(b)
  BN_set_word(a, UINT64_MAX);
  BN_set_word(b, UINT64_MAX);
  BIGINT *r;
  BN_alloca(r)
  BN_mul(r, a, b);
  BN_print(r);
  return 0;
}
