#ifndef MYFLPT_H
#define MYFLPT_H 

#include <stdint.h>
#define MYFLPT_G 8 // The number of bits allowed to calculate the true sum.
//! \brief My floating point format: 31: sign; 30-8 (23 bits): mantissa; 7-0 (8 bits): exponent in excess -250.
typedef uint32_t (myflpt);

// Convert an IEEE 754 float to myflpt
static inline myflpt float_to_myflpt(float f) {

    union { float f; uint32_t u; } conv = { .f = f };
    uint32_t bits = conv.u;

    uint32_t sign = bits >> 31;
    uint32_t exp  = (bits << 1) >> 24;
    if (exp == 0) return 0;                           // If exp == 0, then f == 0 so return zero. Without this, the function would return 0.5 × 2^(-126)

    uint32_t mant = (bits << 9) >> 9;

    uint32_t e = exp + 124;         // 1.m*2^(exp-127) = 0.1m*2^(exp-126) = 0.1m*2^(exp+124-250) = 0.1m*2^(e-250)
    if (e > 255) return (sign << 31) | 0x7FFFFFFF;             // If e overflows, then return the largest possible value +/- 2^5 = 32

    uint32_t m = ((1 << 23) | mant) >> 1;

    return (sign << 31) | (m << 8) | e;
}

static inline void order(myflpt *a, myflpt *b) {
    uint32_t exp_a = (*a << 24) >> 24;
    uint32_t exp_b = (*b << 24) >> 24;

    uint32_t mant_a = (*a << 1) >> 9;
    uint32_t mant_b = (*b << 1) >> 9;

    if (exp_b > exp_a || (exp_b == exp_a && mant_b > mant_a)) {
        uint32_t tmp = *a;
        *a = *b;
        *b = tmp;
    }
}

static inline myflpt add(myflpt a, myflpt b) {

    if (a == 0) return b;
    if (b == 0) return a;

    order(&a, &b);

    // Extract each sign, exponent and mantissa
    uint32_t sign_a = (a >> 31);
    uint32_t sign_b = (b >> 31);

    uint32_t exp_a = (a << 24) >> 24;
    uint32_t exp_b = (b << 24) >> 24;

    uint32_t mant_a = ((a << 1) >> 9) << MYFLPT_G;
    uint32_t mant_b = ((b << 1) >> 9) << MYFLPT_G;

    uint32_t s = sign_a;
    int32_t e = exp_a;
    uint32_t m;

    uint32_t d = exp_a - exp_b;
    if (d > (23 + MYFLPT_G)) return a;               // b is too small to be added to a
    else mant_b = mant_b >> d;

    // Compute each component
    if (sign_a == sign_b) {                         // negative + negative / postive + positive
        m = mant_a + mant_b;
        if (m >= ((uint32_t)1 << (23 + MYFLPT_G))) {         // If m >= 1.0, then shift m and increment e.
            m = m >> 1;
            e += 1;
        }
    } else {                                        // positive + negative
        m = mant_a - mant_b;
        if (m == 0) return 0;
        while (m < ((uint32_t)1 << (22 + MYFLPT_G))) {        // shift until leading 1 is back at bit 22 + the guard.
            m = m << 1;
            e -= 1;
        }
    }

    if (e > 255) return (s << 31) | 0x7FFFFFFF;             // If e overflows, then return the largest possible value +/- 2^5 = 32
    if (e < 0) return 0;                                    // If e underflows, then flush to 0.
    m = m >> MYFLPT_G;

    return (s << 31) | (m << 8) | e;
}

static inline myflpt sub(myflpt a, myflpt b) {
    if (b == 0) return a;

    // a - b = a + (-b)
    return add(a, b ^ 0x80000000);
}

static inline myflpt mul(myflpt a, myflpt b) {

    if (a == 0 || b == 0) return 0;                 // If a or b == 0, then return zero.

    // Extract each sign, exponent and mantissa
    uint32_t sign_a = (a >> 31);
    uint32_t sign_b = (b >> 31);

    uint32_t exp_a = (a << 24) >> 24;
    uint32_t exp_b = (b << 24) >> 24;

    uint32_t mant_a = (a << 1) >> 9;
    uint32_t mant_b = (b << 1) >> 9;

    // Compute each component
    uint32_t s = sign_a ^ sign_b;
    int32_t e = exp_a + exp_b - 250;

    // Product of mantissa
    uint64_t prod = (uint64_t) mant_a * mant_b;
    uint32_t m;

    if (prod >= ((uint64_t) 1 << 45)) {       // product >= 0.5: 0.1~
        m = (uint32_t)(prod >> 23);
    } else {                                  // product < 0.5: 0.01~
        m = (uint32_t)(prod >> 22);                       
        e -= 1;
    }

    if (e > 255) return (s << 31) | 0x7FFFFFFF;             // If e overflows, then return the largest possible value +/- 2^5 = 32
    if (e < 0) return 0;                                    // If e underflows, then flush to 0.

    return (s << 31) | (m << 8) | e;
}

static inline int is_smaller(myflpt a, myflpt b) {
    uint32_t exp_a = (a << 24) >> 24;
    uint32_t exp_b = (b << 24) >> 24;

    uint32_t mant_a = (a << 1) >> 9;
    uint32_t mant_b = (b << 1) >> 9;

    return (exp_a < exp_b) || (exp_a == exp_b && mant_a < mant_b);
}

#endif // MYFLPT_H