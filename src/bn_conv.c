#include "bn.h"
#include <ctype.h>

// lookup table
const char Hex[] = "0123456789ABCDEF";

/// Parses a big decimal number into a bigint
/// \param bn A pointer to a pointer to a bigint,
/// the pointer that it points to can be a NULL pointer
/// \param a The decimal number as a c string
/// \return The number of characters read
int BN_dec2bn(BIGINT **bn, const char *a) {
  BIGINT *ret = NULL;
  int neg = 0, i, j;
  int num;

  if (a == NULL || *a == '\0')
    return 0;
  if (*a == '-') {
    neg = 1;
    a++;
  }

  for (i = 0; i <= INT_MAX / 4 && isdigit(a[i]); i++)
    continue;

  if (i == 0 || i > INT_MAX / 4)
    goto err;

  num = i + neg;
  if (bn == NULL)
    return num;

  /*
   * a is the start of the digits, and it is 'i' long. We chop it into
   * BN_DEC_NUM digits at a time
   */
  if (*bn == NULL) {
    if ((ret = BN_new()) == NULL)
      return 0;
  } else {
    ret = *bn;
    BN_zero(ret);
  }

  /* i is the number of digits, a bit of an over expand */
  if (bn_expand(ret, i * 4) == NULL)
    goto err;

  j = BN_DEC_NUM - i % BN_DEC_NUM;
  if (j == BN_DEC_NUM)
    j = 0;
  u64 l = 0;
  while (--i >= 0) {
    l *= 10;
    l += *a - '0';
    a++;
    if (++j == BN_DEC_NUM) {
      if (!BN_mul_word(ret, BN_DEC_CONV) || !BN_add_word(ret, l))
        goto err;
      l = 0;
      j = 0;
    }
  }

  bn_correct_top(ret);
  *bn = ret;
  /* Don't set the negative flag if it's zero. */
  if (ret->top != 0)
    ret->neg = neg;
  return num;
err:
  if (*bn == NULL)
    BN_free(ret);
  return 0;
}

/// Formats a bigint
/// \param a The bigint to format
/// \return A pointer to the formatted string, the caller is responsible of freeing the string
char *BN_bn2dec(const BIGINT *a) {
  int num, ok = 0, n, tbytes;
  char *buf = NULL;
  char *p;
  BIGINT *t = NULL;
  u64 *bn_data = NULL, *lp;
  int bn_data_num;
  /*-
   * get an upper bound for the length of the decimal integer
   * num <= (BN_num_bits(a) + 1) * log(2)
   *     <= 3 * BN_num_bits(a) * 0.101 + log(2) + 1     (rounding error)
   *     <= 3 * BN_num_bits(a) / 10 + 3 * BN_num_bits / 1000 + 1 + 1
   */
  int i = BN_num_bits(a) * 3;
  num = (i / 10 + i / 1000 + 1) + 1;
  tbytes = num + 3; /* negative and terminator and one spare? */
  bn_data_num = num / BN_DEC_NUM + 1;
  bn_data = malloc(bn_data_num * sizeof(u64));
  buf = malloc(tbytes);
  if (buf == NULL || bn_data == NULL) {
    goto err;
  }
  if ((t = BN_dup(a)) == NULL)
    goto err;

  p = buf;
  lp = bn_data;
  if (BN_is_zero(t)) {
    *p++ = '0';
    *p = '\0';
  } else {
    if (BN_is_negative(t))
      *p++ = '-';

    while (!BN_is_zero(t)) {
      if (lp - bn_data >= bn_data_num)
        goto err;
      *lp = BN_div_word(t, BN_DEC_CONV);
      if (*lp == (u64)-1)
        goto err;
      lp++;
    }
    lp--;
    /*
     * We now have a series of blocks, BN_DEC_NUM chars in length, where
     * the last one needs truncation. The blocks need to be reversed in
     * order.
     */
    n = snprintf(p, tbytes - (size_t)(p - buf), BN_DEC_FMT1, *lp);
    if (n < 0)
      goto err;
    p += n;
    while (lp != bn_data) {
      lp--;
      n = snprintf(p, tbytes - (size_t)(p - buf), BN_DEC_FMT2, *lp);
      if (n < 0)
        goto err;
      p += n;
    }
  }
  ok = 1;
err:
  free(bn_data);
  BN_free(t);
  if (ok)
    return buf;
  free(buf);
  return NULL;
}

char *BN_bn2hex(const BIGINT* a) {
  int v, z;
  if (BN_is_zero(a)) {
    char *buf = malloc(1);
    *buf = '0';
    return buf;
  }

  char *buf = malloc(a->top*BN_BYTES*2+2);
  if (buf == NULL) {
    goto err;
  }
  char *p = buf;
  if (a->neg) {
    *p = '-';
    p++;
  }
  for (int i = a->top - 1; i>=0; i--) {
    for (int j = BN_BITS2 - 8; j >= 0; j-=8) {
      v = (int)((a->d[i]>>j) & 0xff);
      if (z || v != 0) {
        *p++ = Hex[v>>4];
        *p++ = Hex[v&0x0f];
        z = 1;
      }
    }
  }
  *p = '\0';
err:
  return buf;
}