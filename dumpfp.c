// Author: Josh Haberman (jhaberman@gmail.com)
// This code is in the public domain.

#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: dumpfp 0.12345\n");
    return 1;
  }

  double x = atof(argv[1]);

  // We assume that floats and ints have the same endianness.
  uint64_t u64;
  memcpy(&u64, &x, 8);

  uint64_t sign = u64 >> 63,
           exp = (u64 >> 52) & 0x7FF,
           sig = u64 & ((1LL << 52) - 1);
  int16_t exp2 = exp - 1023;

  long double val = 1 + (long double)sig / (1LL << 52);
  __uint128_t product = sig * (__uint128_t)pow(10, 16);
  uint64_t fraction = product / (1LL << 52);
  __uint128_t product2 = (__uint128_t)fraction * (1LL << 52);

  printf("double precision (IEEE 64-bit):\n");
  printf("         val = 0x%" PRIx64 "\n", u64);
  printf("        sign = 0x%" PRIx64 "\n", sign);

  if (exp2 == 1024) {
    printf("    exponent = 0x%" PRIx64 " (NaN or Infinity)\n", exp);
    if (sig)
      printf(" significand = %" PRIx64 " (non-zero indicates NaN)\n", sig);
    else
      printf(" significand = %" PRIx64 " (zero indicates Infinity)\n", sig);
  } else {
    printf("    exponent = 0x%" PRIx64 " (%d)\n", exp, exp2);
    if (product2 == product)
      printf(" significand = %" PRIx64 " (1.%" PRIu64 ")\n", sig, fraction);
    else
      printf(" significand = %" PRIx64 " (between 1.%" PRIu64 " and 1.%"
             PRIu64 ")\n", sig, fraction, fraction+1);
    printf("\n");
    // We don't have the ability to print the result exactly without
    // arbitrary-precision integer arithmetic.
    printf("  1.%" PRIu64 " * 2**%d ~ %Lg\n", fraction, exp2, val * pow(2, exp2));
  }

  printf("\n");

  return 0;
}
