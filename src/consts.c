#include <stdint.h>
#include "bn.h"

const int BN_DEC_NUM = 19;
const u64 BN_DEC_CONV = UINT64_C(10000000000000000000);

const int BN_BYTES = 8;
const int BN_BITS2 = BN_BYTES * 8;
const int BN_BITS4 = 32;

#ifndef ASM
const long BN_MASK2l = 0xffffffffL;
const unsigned long long BN_MASK2h = 0xffffffff00000000LL;
#endif

const char *BN_DEC_FMT1 = "%lu";
const char *BN_DEC_FMT2 = "%019lu";

const u64 one_d[1] = {1};
const BIGINT C_BN_one = { .d = &one_d, .top = 1, .dmax = 1, .neg = 0 };
const BIGINT C_BN_zero = { .d = NULL, .top = 0, .dmax = 0, .neg = 0};
