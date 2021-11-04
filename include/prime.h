#include "bn.h"

typedef BIGINT MONT;

struct mont_params_t {
  BIGINT R;
  BIGINT RR;
  BIGINT N;
  BIGINT Ni;
  MONT One;
};

struct rng_params_t {
  int top;
  u64 *d;
  u64 hd_used_bits;
};

typedef struct mont_params_t MONT_PARAMS;
typedef struct rng_params_t RNG_PARAMS;

MONT_PARAMS *init_mont(const BIGINT *n);
void free_mont(MONT_PARAMS *params);
bool REDC(BIGINT *r, const BIGINT *t, const MONT_PARAMS *params);
bool to_mont(MONT *r, const BIGINT *a, const MONT_PARAMS *params);
#define from_mont(r, t, params) REDC(r, t, params)

bool MONT_mul(MONT *r, const MONT *a, const MONT *b, const MONT_PARAMS *params);
bool MONT_exp(MONT *r, const MONT *a, const BIGINT *e, const MONT_PARAMS *params);
bool BN_mod_exp(BIGINT *r, const BIGINT *a, const BIGINT *e, const BIGINT *m);
bool BN_mod_exp_mont(BIGINT *r, const BIGINT *a, const BIGINT *e, const MONT_PARAMS *params);

void seed(void);
u64 next(void);
RNG_PARAMS *to_rng_params(const BIGINT *a, bool copy);
bool bn_gen(BIGINT *r, const RNG_PARAMS *params);
bool bn_gen_pred(BIGINT *r, const RNG_PARAMS *params, bool (*pred)(const BIGINT *));

bool extgcd(BIGINT *gcd, BIGINT *coe_x, BIGINT *coe_y,
            const BIGINT *a, const BIGINT *b);
