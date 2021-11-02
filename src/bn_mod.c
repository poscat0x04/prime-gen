#include "bn.h"
#include "assert.h"

/// Computes m modular d and places the remainder in r
/// \param r A bigint to hold the return value
/// \param m The input
/// \param d The modulus
/// \return Whether the operations is successful
bool BN_mod(BIGINT *r, const BIGINT *m, const BIGINT *d) {
  if (!(BN_div(NULL, r, m, d)))
    return false;
  if (!r->neg)
    return true;
  return (d->neg ? BN_sub : BN_add)(r, r, d);
}

bool BN_mod_sqr(BIGINT *r, const BIGINT *a, const BIGINT *m)
{
  if (!BN_sqr(r, a))
    return false;
  return BN_mod(r, r, m);
}

/// Computes m mod 2^64e
/// \param r A bigint to hold the return value
/// \param m The input
/// \param e The exponent
bool BN_mod_digits(BIGINT *r, const BIGINT *m, int e) {
  assert(e > 0);
  if (r != m)
    if (BN_copy(r, m) == NULL)
      return false;
  r->top = e;
  return true;
}
