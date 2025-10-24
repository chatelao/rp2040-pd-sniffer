#ifndef PD_COMMON_H
#define PD_COMMON_H

#ifdef NATIVE_BUILD
#include <stdint.h>
uint64_t time_us_64(void);
#else
#include "pico/stdlib.h"
#endif

#endif // PD_COMMON_H
