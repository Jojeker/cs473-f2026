#ifndef FXPT_H
#define FXPT_H

#define FXPT_FRAC 24 // In Q(32-FXPT_FRAC).FXPT_FRAC
#define FX(x) ((fxpt)((x) * (1 << FXPT_FRAC))) // Convert float x into fxpt FX(x)

#include <stdint.h>
//! \brief Signed Q8.24 fixed-point type
typedef int32_t (fxpt);

static inline fxpt mul(fxpt a, fxpt b) {
    return (fxpt)(((int64_t) a * b) >> FXPT_FRAC);
}

#endif // FXPT_H