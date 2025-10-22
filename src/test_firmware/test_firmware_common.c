#include "test_firmware_common.h"

// Custom VDM for testing: "PING"
const uint32_t PING_VDM_HEADER = 0x0001;
const uint32_t PING_VDM_DATA[] = {0x50494E47}; // "PING"

// Custom VDM for testing: "PONG"
const uint32_t PONG_VDM_HEADER = 0x0001;
const uint32_t PONG_VDM_DATA[] = {0x504F4E47}; // "PONG"

// 5V/3A Fixed Supply PDO
const uint32_t SOURCE_CAP_PDO = (1 << 28) | (1 << 26) | (1 << 25) | (100 << 10) | 150;
