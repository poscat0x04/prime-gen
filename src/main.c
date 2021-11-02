#include "bn.h"
#include <alloca.h>
#include <string.h>

int main(void) {
  BIGINT *a;
  BN_alloca(a)
  BN_set_word(a, UINT64_MAX);
  BIGINT *r;
  BN_alloca(r)
  BN_sqr(r, a);
  BN_print(r);
  BN_free_allocas(2, r, a);
  return 0;
}
