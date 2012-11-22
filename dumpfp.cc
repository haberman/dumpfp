// Author: Josh Haberman (jhaberman@gmail.com)
// This code is in the public domain.

#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <iomanip>
#include <gmpxx.h>

using namespace std;

mpz_class bitat(int bit) { return mpz_class(1) << bit; }
mpz_class mask(int bits) { return bitat(bits) - 1; }

void dumpfp(mpz_class whole, mpz_class numer, int denom_lg2) {
  mpz_class denom = bitat(denom_lg2);

  // Reduce the fraction for nicer display.
  while ((numer & 0x1) == 0 && (denom & 0x1) == 0) {
    numer >>= 1;
    denom >>= 1;
    denom_lg2--;
  }

  // Pick the fewest number of digits that will precisely represent the value.
  mpz_class digits = 10;
again:
  mpz_class product = numer * digits;
  mpz_class fraction = product / denom;
  mpz_class product2 = fraction * denom;
  if (product2 != product) {
    digits *= 10;
    goto again;
  }

  if (whole > 0)
    cout << whole << " + ";
  cout << numer << "/2^" << denom_lg2;
  cout << "  (" << whole << "." << fraction << ")";
}

void dumpfp2(mpz_class raw, int sig_bits, int exp_bits) {
  // In these calculations we rely on the general properties of IEEE floating
  // point: X bits of significant, followed by an implicit "1", followed by
  // Y bits of exponent and a sign bit.  The exponent is expected to be biased
  // by half of the exponent's domain.
  //
  // Note that this will *not* work for "long double" because it does not use
  // an implicit 1 in its significand.
  mpz_class sig_scale = bitat(sig_bits),
            sig = raw & mask(sig_bits),
            sig2 = sig_scale + sig,
            expbias = mask(exp_bits - 1);
  int sign = mpz_class(raw >> (sig_bits + exp_bits)).get_si(),
      exp  = mpz_class(raw >> sig_bits & mask(exp_bits)).get_si(),
      exp2 = mpz_class(exp - expbias).get_si();

  cout << "           raw = 0x" << hex << raw << "\n";
  cout << "          sign = 0x" << hex << sign << "\n";

  if (exp2 == (expbias + 1)) {
    cout << "      exponent = 0x" << hex << exp << " (NaN or Infinity)\n";
    if (sig != 0)
      cout << "   significand = " << hex << sig << " (non-zero indicates NaN)\n";
    else
      cout << "   significand = " << hex << sig << " (zero indicates Infinity)\n";
  } else {
    cout << "      exponent = 0x" << hex << exp << " (" << dec << exp2 << ")\n";
    cout << "   significand = 0x" << hex << sig << dec << "\n";
    cout << "\n";

    cout << "   VALUE CALCULATION =\n";
    cout << "       significand   (";
    dumpfp(1, sig, sig_bits);
    cout << ")\n";
    cout << "     * 2^exponent    (2^" << exp2 << ")\n";
    cout << "     = VALUE         (";

    if (exp2 >= sig_bits) {
      cout << (sig2 << (exp2 - sig_bits));
    } else {
      int split = sig_bits - exp2;
      mpz_class whole = sig2 >> split;
      mpz_class frac = sig2 - (whole << split);
      dumpfp(whole, frac, split);
    }
    cout << ")\n";
  }

  cout << "\n";
}

// Gets an integer value corresponding to the given raw bytes.
// The currently will only work on a little-endian machine.
mpz_class getraw(void *val, size_t size) {
  unsigned char bytes[128];
  memcpy(bytes, val, size);
  mpz_class raw(0);
  for (int i = 0; i < size; i++) {
    raw *= 256;
    raw += bytes[size-i-1];
  }
  return raw;
}

void dumpfloat(float x) {
  printf("Single Precision (IEEE 32-bit):\n");
  dumpfp2(getraw(&x, sizeof(float)), 23, 8);
}

void dumpdouble(double x) {
  printf("Double Precision (IEEE 64-bit):\n");
  dumpfp2(getraw(&x, sizeof(double)), 52, 11);
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: dumpfp 0.12345\n");
    return 1;
  }

  long double x = strtold(argv[1], NULL);

  dumpfloat(x);
  dumpdouble(x);

  return 0;
}
