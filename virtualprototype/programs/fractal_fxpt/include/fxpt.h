#ifndef FXPT_H
#define FXPT_H

#include <stdint.h>
//! \brief Pointer to fractal point calculation function
typedef int32_t (fxpt_8_24);

inline fxpt_8_24 float_to_fxpt(float x);
inline fxpt_8_24 mul(fxpt_8_24 a, fxpt_8_24 b);

#endif // FXPT_H