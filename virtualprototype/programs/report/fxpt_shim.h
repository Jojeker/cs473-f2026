#ifndef FXPT_SHIM_H
#define FXPT_SHIM_H
#include <stdint.h>
#define FXPT_FRAC 24
#define FX(x) ((fxpt)((x) * (1 << FXPT_FRAC)))
typedef int32_t fxpt;
static inline fxpt fxmul(fxpt a, fxpt b){ return (fxpt)(((int64_t)a*b)>>FXPT_FRAC); }
#endif
