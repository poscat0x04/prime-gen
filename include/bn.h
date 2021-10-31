#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

// floor(log10(2^64))
#define BN_DEC_NUM 19
// 10^BN_DEC_NUM
#define BN_DEC_CONV (UINT64_C(10000000000000000000))

#define BN_BYTES 8
#define BN_BITS2 (BN_BYTES * 8)
#define BN_BITS4 32

#define BN_MASK2 (0xffffffffffffffffLL)
#define BN_MASK2l (0xffffffffL)
#define BN_MASK2h (0xffffffff00000000LL)

#define LBITS(a) ((a)&BN_MASK2l)
#define HBITS(a) (((a) >> BN_BITS4) & BN_MASK2l)
#define L2HBITS(a) (((a) << BN_BITS4) & BN_MASK2)

#define BN_DEC_FMT1 "%lu"
#define BN_DEC_FMT2 "%019lu"

#define mul64(l, h, bl, bh)                                                    \
  {                                                                            \
    u64 m, m1, lt, ht;                                                         \
                                                                               \
    lt = l;                                                                    \
    ht = h;                                                                    \
    m = (bh) * (lt);                                                           \
    lt = (bl) * (lt);                                                          \
    m1 = (bl) * (ht);                                                          \
    ht = (bh) * (ht);                                                          \
    m = (m + m1) & BN_MASK2;                                                   \
    if (m < m1)                                                                \
      ht += L2HBITS((u64)1);                                                   \
    ht += HBITS(m);                                                            \
    m1 = L2HBITS(m);                                                           \
    lt = (lt + m1) & BN_MASK2;                                                 \
    if (lt < m1)                                                               \
      ht++;                                                                    \
    (l) = lt;                                                                  \
    (h) = ht;                                                                  \
  }

#define mul(r, a, bl, bh, c)                                                   \
  {                                                                            \
    u64 l, h;                                                                  \
                                                                               \
    h = (a);                                                                   \
    l = LBITS(h);                                                              \
    h = HBITS(h);                                                              \
    mul64(l, h, (bl), (bh));                                                   \
                                                                               \
    /* non-multiply part */                                                    \
    l += (c);                                                                  \
    if ((l & BN_MASK2) < (c))                                                  \
      h++;                                                                     \
    (c) = h & BN_MASK2;                                                        \
    (r) = l & BN_MASK2;                                                        \
  }

typedef uint64_t u64;
typedef int64_t i64;
typedef uint32_t u32;
typedef int32_t i32;

struct bignum_st {
  u64 *d;   /* pointer to an array of u64 chunks */
  int top;  /* index of last used d +1 */
  int dmax; /* size of the d array */
  bool neg; /* whether if the number is negative */
};

typedef struct bignum_st BIGNUM;

u64 *bn_expand_internal(BIGNUM *b, int word);
BIGNUM *bn_expand2(BIGNUM *b, int word);
BIGNUM *bn_wexpand(BIGNUM *a, int word);
static inline BIGNUM *bn_expand(BIGNUM *a, int bits) {
  if (bits > (INT_MAX - BN_BITS2 + 1))
    return NULL;

  if (((bits + BN_BITS2 - 1) / BN_BITS2) <= (a)->dmax)
    return a;

  return bn_expand2((a), (bits + BN_BITS2 - 1) / BN_BITS2);
}

void BN_free(BIGNUM *bn);
void BN_init(BIGNUM *bn);
BIGNUM *BN_new(void);
BIGNUM *BN_dup(const BIGNUM *a);
BIGNUM *BN_copy(BIGNUM *a, const BIGNUM *b);

bool BN_set_word(BIGNUM *a, u64 w);
#define BN_zero(a) (BN_set_word((a), 0))
void BN_set_negative(BIGNUM *a, bool neg);

int BN_dec2bn(BIGNUM **bn, const char *str);
char *BN_bn2dec(const BIGNUM *a);

// set dmax to point to the last non-zero digit
void bn_correct_top(BIGNUM *a);

bool BN_add_word(BIGNUM *a, u64 w);
bool BN_sub_word(BIGNUM *a, u64 w);
bool BN_mul_word(BIGNUM *a, u64 w);
u64 BN_div_word(BIGNUM *a, u64 w);

bool BN_uadd(BIGNUM *r, BIGNUM *a, BIGNUM *b);
bool BN_add(BIGNUM *r, BIGNUM *a, BIGNUM *b);
bool BN_mul(BIGNUM *r, BIGNUM *a, BIGNUM *b);
bool BN_div(BIGNUM *r, BIGNUM *a, BIGNUM *b);

bool BN_lshift(BIGNUM *r, const BIGNUM *a, int n);

int BN_num_bits(const BIGNUM *a);
int BN_num_bits_word(u64 w);

bool BN_is_zero(const BIGNUM *a);
bool BN_is_negative(const BIGNUM *a);

u64 bn_mul_words(u64 *rp, const u64 *ap, int num, u64 w);
u64 bn_div_words(u64 h, u64 l, u64 d);
bool bn_is_odd(const BIGNUM *a);
bool bn_lshift_fixed_top(BIGNUM *r, const BIGNUM *a, int n);
