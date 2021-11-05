#include "bn.h"

/// Prints a bignum to a file descriptor
/// \param a The bignum to print
/// \param file The file descriptor to print to
/// \param conv The conversion function
/// \return Whether the operation is successful
bool BN_print_file(const BIGINT *a, FILE *file, char *(*conv) (const BIGINT *)) {
  char *str;
  if ((str = conv(a)) == NULL) {
    return false;
  } else {
    fputs(str, file);
    free(str);
    return true;
  }
}

/// Prints a bignum to stdout
/// \param a The bignum to print
/// \param conv The conversion function
/// \return Whether the operation is successful
bool BN_print(const BIGINT *a, char *(*conv) (const BIGINT *)) {
  return BN_print_file(a, stdout, conv);
}

/// Prints a bitnum and a line break to stdout
bool BN_println(const BIGINT *a, char*(*conv) (const BIGINT *)) {
  if (BN_print_file(a, stdout, conv)) {
    putchar('\n');
    return true;
  } else
    return false;
}

bool BN_dprint(const BIGINT *a) {
  return BN_print(a, BN_bn2dec);
}

bool BN_hprint(const BIGINT *a) {
  return BN_print(a, BN_bn2hex);
}

bool BN_dprintln(const BIGINT *a) {
  return BN_println(a, BN_bn2dec);
}

bool BN_hprintln(const BIGINT *a) {
  return BN_println(a, BN_bn2hex);
}
