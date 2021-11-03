#include "bn.h"

struct mont_params_t {
  BIGINT R;
  BIGINT RR;
  BIGINT N;
  BIGINT Ni;
};

typedef struct mont_params_t MONT_PARAMS;

MONT_PARAMS *init_mont(const BIGINT *n);
void free_mont(MONT_PARAMS *params);
bool REDC(BIGINT *r, const BIGINT *t, MONT_PARAMS *params);

bool extgcd(BIGINT *gcd, BIGINT *coe_x, BIGINT *coe_y,
            const BIGINT *a, const BIGINT *b);
