#include "bn.h"
#include <alloca.h>
#include <string.h>
#include <assert.h>

int main(void) {
  BN_init(a)
  BN_set_word(a, UINT64_MAX);
  BN_init(r)
  BN_sqr(r, a);
  BN_print(r);
  putchar('\n');
  BN_set_word(a, 1);
  BN_lshift_digits(a, a, 1);
  BN_print(a);
  BN_free_allocas(2, r, a);
  return 0;
}
