#include "prime.h"
#include "alloca.h"
#include "string.h"
#include "assert.h"

bool extgcd(BIGINT *gcd, BIGINT *coe_x, BIGINT *coe_y, const BIGINT *a, const BIGINT *b) {
  assert(!BN_is_negative(a));
  assert(!BN_is_negative(b));

  bool res = false;
  bool swapped = false;

  // ensures a >= b
  if (BN_ucmp(a, b) < 0) {
    swapped = true;
    const BIGINT *tmp = a;
    a = b;
    b = tmp;
  }

  BN_init(r)
  BN_init(old_r)
  BN_init(x)
  BN_init(old_x)
  BN_init(y)
  BN_init(tmp1)
  BN_init(tmp2)
  if (BN_copy(old_r, a) == NULL
      || BN_copy(r, b) == NULL)
    goto end;
  if (!BN_set_word(old_x, 1)
      || !BN_set_word(x, 0))
    goto end;

  while (!BN_is_zero(r)) {
    // tmp1: q, tmp2: new_r
    if (!BN_div(tmp1, tmp2, old_r, r)
        || !BN_move(old_r, r)
        || !BN_move(r, tmp2)
        || !BN_mul(tmp2, tmp1, x)
        || !BN_sub(tmp1, old_x, tmp2)
        || !BN_move(old_x, x)
        || !BN_move(x, tmp1))
      goto end;
  }

  if (gcd != NULL && BN_copy(gcd, old_r) == NULL)
    goto end;
  if (!BN_mul(tmp1, old_x, a)
      || !BN_sub(tmp2, old_r, tmp1))
    goto end;
  if (BN_is_zero(b)) {
    if (!BN_set_word(y, 0))
      goto end;
  } else {
    if (!BN_div(y, NULL, tmp2, b))
      goto end;
  }

  if (swapped) {
    if ((coe_y != NULL && BN_move(coe_y, old_x) == NULL)
        || (coe_x != NULL && BN_move(coe_x, y) == NULL))
      goto end;
  } else {
    if ((coe_x != NULL && BN_move(coe_x, old_x) == NULL)
        || (coe_y != NULL && BN_move(coe_y, y) == NULL))
      goto end;
  }
  res = true;
end:
  BN_free_allocas(6, r, old_r, x, old_x, y, tmp1, tmp2);
  return res;
}
