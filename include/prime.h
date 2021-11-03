#include "bn.h"

struct mont_params_t {
  BIGINT R;
  BIGINT RR;
  BIGINT N;
  BIGINT Ni;
};

typedef struct mont_params_t MONT_PARAMS;
typedef BIGINT MONT;

MONT_PARAMS *init_mont(const BIGINT *n);
void free_mont(MONT_PARAMS *params);
bool REDC(BIGINT *r, const BIGINT *t, const MONT_PARAMS *params);
bool to_mont(MONT *r, const BIGINT *a, const MONT_PARAMS *params);
#define from_mont(r, t, params) REDC(r, t, params)

bool extgcd(BIGINT *gcd, BIGINT *coe_x, BIGINT *coe_y,
            const BIGINT *a, const BIGINT *b);
