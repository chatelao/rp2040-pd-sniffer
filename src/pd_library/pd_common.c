#include "pd_common.h"

#ifdef NATIVE_BUILD
#include <sys/time.h>
#include <stddef.h>
uint64_t time_us_64(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000 + tv.tv_usec;
}
#endif
