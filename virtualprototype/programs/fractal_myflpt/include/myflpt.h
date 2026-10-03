#ifndef MYFLPT_H
#define MYFLPT_H

#define FL(x) ((fxpt)((x) * (1 << FXPT_FRAC))) // Converts float x into fxpt


#include <stdint.h>
//! \brief My floating point format: 31: sign; 30-8 (23 bits): mantissa; 7-0 (8 bits): exponent in excess -250.
typedef int32_t (myflpt);

// Convert an IEEE 754 float to myflpt
static inline myflpt float_to_myflpt(float f) {
    union { float f; uint32_t u; } conv = { .f = f };
    uint32_t bits = conv.u;

    uint32_t sign = bits >> 31;
    uint32_t exp  = (bits << 1) >> 24;
    uint32_t mant = (bits << 9) >> 9;

    if (exp == 0) return 0;                            // zero (tiny denormals become zero too)

    uint32_t e = exp + 124;                            // 1.m*2^(exp-127) = 0.1m*2^(exp-126) = 0.1m*2^(exp+124-250) = 0.1m*2^(e-250)
    uint32_t m = ((1 << 23) | mant) >> 1;            // put the hidden 1 back, then 0.1m in 23 bits

    return sign | (m << 8) | e;
}

static inline fxpt mul(fxpt a, fxpt b) {
    return (fxpt)(((int64_t) a * b) >> FXPT_FRAC);
}

#endif // MYFLPT_H