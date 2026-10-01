#ifndef FXPT_H
#define FXPT_H

#include <stdint.h>
//! \brief Signed Q8.24 fixed-point type
typedef int32_t (fxpt_8_24);

static inline fxpt_8_24 float_to_fxpt(float x) {
    return (fxpt_8_24)(x * (1 << 24));
}

static inline fxpt_8_24 mul(fxpt_8_24 a, fxpt_8_24 b) {
    return (fxpt_8_24)(((int64_t) a * b) >> 24);
}

static inline float fxpt_to_float(fxpt_8_24 a) {
    return (float)(a * (1 >> 24));
}

#endif // FXPT_H