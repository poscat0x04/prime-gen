#define _GNU_SOURCE
#include "prime.h"
#include <alloca.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

const int ITER = 10;

int gen(const char *filename) {
  FILE *result = fopen(filename, "w+");
  if (result == NULL) {
    fprintf(stderr, "failed to open %s: errno %d\n", filename, errno);
    return 1;
  }
  BN_init(a)
  if (!BN_gen_prime_bits(a, ITER, 1024)) {
    fputs("internal error: failed to generate a prime\n", stderr);
    return 127;
  }
  BN_print_file(a, result, BN_bn2dec);
  if (fclose(result) != 0) {
    fprintf(stderr, "failed to close result.txt: errno %d\n", errno);
    return 1;
  }
  return 0;
}

int test(char *filename) {
  FILE *test = fopen(filename, "r");
  if (test == NULL) {
    fprintf(stderr, "failed to open %s: errno %d\n", filename, errno);
    return 1;
  }
  BN_init(a)
  size_t len = 0;
  int r;
  char *line = NULL;
  int line_num = 0;
  bool is_prime;
  while ((len = getline(&line, &len, test)) != EOF) {
    line_num ++;
    if (len == 1)
      continue;
    r = BN_dec2bn(&a, line);
    if (r == 0) {
      fprintf(stderr, "unable to parse line %d\n", line_num);
      puts(line);
      return 1;
    }
    if (!BN_is_prime(a, ITER, &is_prime))
      return 127;
    printf("%s\n", is_prime ? "True" : "False");
  }
  return 0;
}

int interactive_test(void) {
  char *line = NULL;
  BN_init(tmp)
  size_t len = 0;
  puts("Input a decimal number: ");
  while ((len = getline(&line, &len, stdin)) == 1);
  int r = BN_dec2bn(&tmp, line);
  if (r == 0 || r != len - 1) {
    fputs("Failed to parse number\n", stderr);
    return 1;
  }
  bool is_prime = false;
  if (!BN_is_prime(tmp, ITER, &is_prime))
    return 127;
  printf("%s\n", is_prime ? "True" : "False");
  return 0;
}

int main(int argc, char *argv[]) {
  seed();
  if (argc == 1) {
    return gen("result.txt");
  } else if (argc >= 2) {
    if (strcmp(argv[1], "test") == 0) {
      if (argc >= 3) {
        if (strcmp(argv[2], "-f") == 0) {
          if (argc >= 4) {
            return test(argv[3]);
          } else {
            fputs("missing filename for flag \"-f\"\n", stderr);
            return 1;
          }
        } else {
          fprintf(stderr, "unknown option \"%s\" for command \"test\"\n", argv[2]);
          return 1;
        }
      } else {
        return interactive_test();
      }
    } else if (strcmp(argv[1], "gen") == 0) {
      if (argc >= 3) {
        if (strcmp(argv[2], "-f") == 0) {
          if (argc >= 4) {
            return gen(argv[3]);
          } else {
            fputs("missing filename for flag \"-f\"\n", stderr);
            return 1;
          }
        } else {
          fprintf(stderr, "unknown option %s for command \"gen\"\n", argv[2]);
          return 1;
        }
      } else {
        gen("result.txt");
      }
    } else {
      fprintf(stderr, "unknown command \"%s\"\n", argv[1]);
      return 1;
    }
  }
  return 0;
}
