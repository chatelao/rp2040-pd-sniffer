#ifndef PD_COMMON_H
#define PD_COMMON_H

#ifdef NATIVE_BUILD
#include <stdint.h>
/**
 * @brief Returns the time in microseconds since boot.
 * @return The time in microseconds since boot.
 */
uint64_t time_us_64(void);
#else
#include "pico/stdlib.h"
#endif

#endif // PD_COMMON_H
