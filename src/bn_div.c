#include "bn.h"
#include <assert.h>
#include <alloca.h>
#include <string.h>

static int bn_left_align(BIGINT *num) {
  u64 *d = num->d, n, m, rmask;
  int top = num->top;
  int rshift = BN_num_bits_word(d[top-1]), lshift, i;

  lshift = BN_BITS2 - rshift;
  rshift %= BN_BITS2;
  rmask = (u64)0 - rshift;
  rmask |= rmask >> 8;

  for (i = 0, m = 0; i < top; i++) {
    n = d[i];
    d[i] = ((n << lshift) | m) ;
    m = (n >> rshift) & rmask;
  }

  return lshift;
}

bool BN_div(BIGINT *dv, BIGINT *rm, const BIGINT *num, const BIGINT *divisor) {
  bool ret;

  if (BN_is_zero(divisor)) {
    return 0;
  }

  /*
   * Invalid zero-padding would have particularly bad consequences so don't
   * just rely on bn_check_top() here (bn_check_top() works only for
   * BN_DEBUG builds)
   */
  if (divisor->d[divisor->top - 1] == 0) {
    return false;
  }

  ret = bn_div_fixed_top(dv, rm, num, divisor);

  if (ret) {
    if (dv != NULL)
      bn_correct_top(dv);
    if (rm != NULL)
      bn_correct_top(rm);
  }

  return ret;
}

bool bn_div_fixed_top(BIGINT *dv,
                      BIGINT *rm,
                      const BIGINT *num,
                      const BIGINT *divisor) {
  int norm_shift, i, j, loop;
  BIGINT *tmp, *snum, *sdiv, *res;
  u64 *resp, *wnum, *wnumtop;
  u64 d0, d1;
  int num_n, div_n, num_neg;
  bool temp_res = false;

  assert(divisor->top > 0 && divisor->d[divisor->top - 1] != 0);

  if (dv == NULL) {
    temp_res = true;
    BN_alloca(res)
  } else {
    res = dv;
  }
  BN_alloca(tmp)
  BN_alloca(snum)
  BN_alloca(sdiv)

  if (!BN_copy(sdiv, divisor))
    goto err;
  norm_shift = bn_left_align(sdiv);
  sdiv->neg = 0;
  /*
   * Note that bn_lshift_fixed_top's output is always one limb longer
   * than input, even when norm_shift is zero. This means that amount of
   * inner loop iterations is invariant of dividend value, and that one
   * doesn't need to compare dividend and divisor if they were originally
   * of the same bit length.
   */
  if (!(bn_lshift_fixed_top(snum, num, norm_shift)))
    goto err;

  div_n = sdiv->top;
  num_n = snum->top;

  if (num_n <= div_n) {
    /* caller didn't pad dividend -> no constant-time guarantee... */
    if (bn_wexpand(snum, div_n + 1) == NULL)
      goto err;
    memset(&(snum->d[num_n]), 0, (div_n - num_n + 1) * sizeof(u64));
    snum->top = num_n = div_n + 1;
  }

  loop = num_n - div_n;
  /*
   * Lets setup a 'window' into snum This is the part that corresponds to
   * the current 'area' being divided
   */
  wnum = &(snum->d[loop]);
  wnumtop = &(snum->d[num_n - 1]);

  /* Get the top 2 words of sdiv */
  d0 = sdiv->d[div_n - 1];
  d1 = (div_n == 1) ? 0 : sdiv->d[div_n - 2];

  /* Setup quotient */
  if (!bn_wexpand(res, loop))
    goto err;
  num_neg = num->neg;
  res->neg = (num_neg ^ divisor->neg);
  res->top = loop;
  resp = &(res->d[loop]);

  /* space for temp */
  if (!bn_wexpand(tmp, (div_n + 1)))
    goto err;
  for (i = 0; i < loop; i++, wnumtop--) {
    u64 q, l0;
    /*
     * the first part of the loop uses the top two words of snum and sdiv
     * to calculate a u64 q such that | wnum - sdiv * q | < sdiv
     */
    u64 n0, n1, rem = 0;

    n0 = wnumtop[0];
    n1 = wnumtop[-1];
    if (n0 == d0)
      q = UINT64_MAX;
    else {                  /* n0 < d0 */
      u64 n2 = (wnumtop == wnum) ? 0 : wnumtop[-2];
      u64 t2l, t2h;

      // divides n0:n1 by d0
      q = bn_div_words(n0, n1, d0);

      u128 ret = (u128) (d1) * (q);
      t2h = ret >> 64;
      t2l = ret;

      while (true) {
        if ((t2h < rem) || ((t2h == rem) && (t2l <= n2)))
          break;
        q--;
        rem += d0;
        if (rem < d0)
          break;      /* don't let rem overflow */
        if (t2l < d1)
          t2h--;
        t2l -= d1;
      }
    }

    l0 = bn_mul_words(tmp->d, sdiv->d, div_n, q);
    tmp->d[div_n] = l0;
    wnum--;
    /*
     * ignore top values of the bignums just sub the two u64 arrays
     * with bn_sub_words
     */
    l0 = bn_sub_words(wnum, wnum, tmp->d, div_n + 1);
    q -= l0;
    /*
     * Note: As we have considered only the leading two u64s in
     * the calculation of q, sdiv * q might be greater than wnum (but
     * then (q-1) * sdiv is less or equal than wnum)
     */
    for (l0 = 0 - l0, j = 0; j < div_n; j++)
      tmp->d[j] = sdiv->d[j] & l0;
    l0 = bn_add_words(wnum, wnum, tmp->d, div_n);
    (*wnumtop) += l0;
    assert((*wnumtop) == 0);

    /* store part of the result */
    *--resp = q;
  }

  snum->neg = num_neg;
  snum->top = div_n;
  if (rm != NULL)
    bn_rshift_fixed_top(rm, snum, norm_shift);
  if (temp_res) {
    BN_free_alloca(res);
  }
  BN_free_allocas(3, tmp, snum, sdiv);
  return true;
err:
  if (temp_res) {
    BN_free_alloca(res);
  }
  BN_free_allocas(3, tmp, snum, sdiv);
  return false;
}
