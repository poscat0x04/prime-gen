#include <limits.h>
#include <stdbool.h>
#include <stdint.h>

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

bool bn_dec2bn(BIGNUM **bn, const char *str);
char *bn_bn2dec(const BIGNUM *a);

void bn_correct_top(BIGNUM *bn);

bool bn_uadd(BIGNUM *r, BIGNUM *a, BIGNUM *b);
bool bn_add(BIGNUM *r, BIGNUM *a, BIGNUM *b);
bool bn_mul(BIGNUM *r, BIGNUM *a, BIGNUM *b);
bool bn_div(BIGNUM *r, BIGNUM *a, BIGNUM *b);

bool bn_is_odd(const BIGNUM *a);
bool bn_is_even(const BIGNUM *a);
