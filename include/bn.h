#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

typedef __uint128_t u128;
typedef uint64_t u64;

/// Multiplies two u64 numbers a, b and stores the higher bit of the result
/// in high, the lower bit in low
#define BN_UMULT_LOHI(low, high, a, b)                                         \
  {                                                                            \
    u128 ret_=(u128) (a) * (b);                                                \
    (high)=ret_>>64;                                                           \
    (low)=ret_;                                                                \
  }

#define mul_add(r, a, w, c)                                                    \
  {                                                                            \
    u64 high, low, ret, tmp=(a);                                               \
    ret =  (r);                                                                \
    BN_UMULT_LOHI(low, high, w, tmp);                                          \
    ret += (c);                                                                \
    (c) =  (ret < (c)) ? 1 : 0;                                                \
    (c) += high;                                                               \
    ret += low;                                                                \
    (c) += (ret < low) ? 1 : 0;                                                \
    (r) =  ret;                                                                \
  }

#define mul(r, a, w, c)                                                        \
  {                                                                            \
    u64 high, low, ret, ta = (a);                                              \
    BN_UMULT_LOHI(low, high, w, ta);                                           \
    ret =  low + (c);                                                          \
    (c) =  high;                                                               \
    (c) += (ret < low) ? 1 : 0;                                                \
    (r) =  ret;                                                                \
  }

#define sqr(r0, r1, a)                                                         \
  {                                                                            \
    u64 tmp=(a);                                                               \
    BN_UMULT_LOHI(r0,r1,tmp,tmp);                                              \
  }


/// Allocates a bigint on stack, intended to be used for temporary bigint variables
#define BN_alloca(val)                                                         \
  {                                                                            \
    (val) = alloca(sizeof(*(val)));                                            \
    memset(val, 0, sizeof(*(val)));                                            \
  }

// floor(log10(2^64))
extern const int BN_DEC_NUM;
// 10^BN_DEC_NUM
extern const u64 BN_DEC_CONV;

extern const int BN_BYTES;
extern const int BN_BITS2;
extern const int BN_BITS4;

#ifndef ASM
extern const long BN_MASK2l;
extern const unsigned long long BN_MASK2h;
#endif

extern const char *BN_DEC_FMT1;
extern const char *BN_DEC_FMT2;

struct bigint_t {
  u64 *d;   /* pointer to an array of u64 chunks */
  int top;  /* highest significant digit */
  int dmax; /* size of the digits array */
  bool neg; /* whether if the number is negative */
};

typedef struct bigint_t BIGINT;

u64 *bn_expand_internal(BIGINT *b, int word);
BIGINT *bn_expand2(BIGINT *b, int word);
BIGINT *bn_wexpand(BIGINT *a, int word);
/// Expands the max size of a bignum
/// \param a The bignum to expand
/// \param bits The size of the bignum, in bits
/// \return The references to the expanded bignum, if successful,
/// NULL otherwise
static inline BIGINT *bn_expand(BIGINT *a, int bits) {
  if (bits > (INT_MAX - BN_BITS2 + 1))
    return NULL;

  if (((bits + BN_BITS2 - 1) / BN_BITS2) <= (a)->dmax)
    return a;

  return bn_expand2((a), (bits + BN_BITS2 - 1) / BN_BITS2);
}

void BN_free(BIGINT *bn);
void BN_free_alloca(BIGINT *bn);
BIGINT *BN_new(void);
BIGINT *BN_dup(const BIGINT *a);
BIGINT *BN_copy(BIGINT *a, const BIGINT *b);

bool BN_set_word(BIGINT *a, u64 w);
#define BN_zero(a) (BN_set_word((a), 0))
void BN_set_negative(BIGINT *a, bool neg);

int BN_dec2bn(BIGINT **bn, const char *str);
char *BN_bn2dec(const BIGINT *a);
char *BN_bn2hex(const BIGINT *a);

bool BN_print_file(const BIGINT *a, FILE *file);
bool BN_print(const BIGINT *a);

// set dmax to point to the last non-zero digit
void bn_correct_top(BIGINT *a);

bool BN_add_word(BIGINT *a, u64 w);
bool BN_sub_word(BIGINT *a, u64 w);
bool BN_mul_word(BIGINT *a, u64 w);
u64 BN_div_word(BIGINT *a, u64 w);

int BN_ucmp(const BIGINT *a, const BIGINT *b);

bool BN_uadd(BIGINT *r, const BIGINT *a, const BIGINT *b);
bool BN_add(BIGINT *r, const BIGINT *a, const BIGINT *b);
bool BN_usub(BIGINT *r, const BIGINT *a, const BIGINT *b);
bool BN_sub(BIGINT *r, const BIGINT *a, const BIGINT *b);
bool BN_mul(BIGINT *r, const BIGINT *a, const BIGINT *b);
bool BN_div(BIGINT *dv, BIGINT *rm, const BIGINT *num, const BIGINT *divisor);

bool BN_mod(BIGINT *r, const BIGINT *m, const BIGINT *d);

bool BN_lshift(BIGINT *r, const BIGINT *a, int n);

int BN_num_bits(const BIGINT *a);
int BN_num_bits_word(u64 w);

bool BN_is_zero(const BIGINT *a);
bool BN_is_negative(const BIGINT *a);

u64 bn_mul_words(u64 *rp, const u64 *ap, int num, u64 w);
u64 bn_mul_add_words(u64 *rp, const u64 *ap, int num, u64 w);
void bn_mul_normal(u64 *r, u64 *a, int na, u64 *b, int nb);
bool bn_mul_fixed_top(BIGINT *r, const BIGINT *a, const BIGINT *b);
u64 bn_div_words(u64 h, u64 l, u64 d);
u64 bn_sub_words(u64 *r, const u64 *a, const u64 *b, int n);
u64 bn_add_words(u64 *r, const u64 *a, const u64 *b, int n);

bool bn_div_fixed_top(BIGINT *dv,
                      BIGINT *rm,
                      const BIGINT *num,
                      const BIGINT *divisor);
bool BN_is_odd(const BIGINT *a);
bool bn_lshift_fixed_top(BIGINT *r, const BIGINT *a, int n);
bool bn_rshift_fixed_top(BIGINT *r, const BIGINT *a, int n);
