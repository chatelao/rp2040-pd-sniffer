/**
 * @file test_firmware_common.h
 * @brief Common definitions for the test firmware.
 */
#ifndef TEST_FIRMWARE_COMMON_H
#define TEST_FIRMWARE_COMMON_H

#include <stdint.h>

/**
 * @brief Custom VDM for testing: "PING"
 */
extern const uint32_t PING_VDM_HEADER;
extern const uint32_t PING_VDM_DATA[];

/**
 * @brief Custom VDM for testing: "PONG"
 */
extern const uint32_t PONG_VDM_HEADER;
extern const uint32_t PONG_VDM_DATA[];

/**
 * @brief 5V/3A Fixed Supply PDO
 */
extern const uint32_t SOURCE_CAP_PDO;

#endif // TEST_FIRMWARE_COMMON_H
