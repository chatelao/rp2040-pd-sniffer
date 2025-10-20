#ifndef TEST_FIRMWARE_COMMON_H
#define TEST_FIRMWARE_COMMON_H

#include <stdint.h>

// Custom VDM for testing: "PING"
extern const uint32_t PING_VDM_HEADER;
extern const uint32_t PING_VDM_DATA[];

// Custom VDM for testing: "PONG"
extern const uint32_t PONG_VDM_HEADER;
extern const uint32_t PONG_VDM_DATA[];

// 5V/3A Fixed Supply PDO
extern const uint32_t SOURCE_CAP_PDO;

#endif // TEST_FIRMWARE_COMMON_H
