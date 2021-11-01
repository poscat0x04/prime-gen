#include "bn.h"

bool BN_print_file(const BIGINT *a, FILE *file) {
  char *str;
  if ((str = BN_bn2dec(a)) == NULL) {
    return false;
  } else {
    fputs(str, file);
    free(str);
    return true;
  }
}

bool BN_print(const BIGINT* a) {
  return BN_print_file(a, stdout);
}