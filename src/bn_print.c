#include "bn.h"

/// Prints a bignum to a file descriptor
/// \param a The bignum to print
/// \param file The file descriptor to print to
/// \return Whether the operation is successful
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

/// Prints a bignum to stdout
/// \param a The bignum to print
/// \return Whether the operation is successful
bool BN_print(const BIGINT *a) {
  return BN_print_file(a, stdout);
}

/// Prints a bitnum and a line break to stdout
bool BN_println(const BIGINT *a) {
  if (BN_print_file(a, stdout)) {
    putchar('\n');
    return true;
  } else
    return false;
}
