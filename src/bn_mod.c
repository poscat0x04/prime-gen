#include "bn.h"

bool BN_mod(BIGINT *r, const BIGINT *m, const BIGINT *d) {
  if (!(BN_div(NULL, r, m, d)))
    return false;
  if(!r->neg)
    return true;
  return (d->neg ? BN_sub : BN_add)(r, r, d);
}
