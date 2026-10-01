#include "fxpt.h"

inline fxpt_8_24 float_to_fxpt(float x) {
    return (fxpt_8_24)(x * (1 << 24));
}

inline fxpt_8_24 mul(fxpt_8_24 a, fxpt_8_24 b) {
    return (fxpt_8_24)(((int64_t) a * b) >> 24);
}