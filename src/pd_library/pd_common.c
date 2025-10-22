#include "pd_common.h"

#ifdef NATIVE_BUILD
#include <sys/time.h>
#include <stddef.h>
#else
#include "pico/time.h"
#endif

uint64_t time_us_64(void) {
#ifdef NATIVE_BUILD
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000 + tv.tv_usec;
#else
    return time_us_64();
#endif
}
