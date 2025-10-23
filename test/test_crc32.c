#include "unity.h"
#include "pd_library.h"

void test_crc32_simple(void) {
    uint8_t data[] = {0x01, 0x02, 0x03, 0x04};
    uint32_t expected_crc = 0xB63CFBCD;
    uint32_t actual_crc = pd_crc32(data, sizeof(data));
    TEST_ASSERT_EQUAL_HEX32(expected_crc, actual_crc);
}

void test_crc32_string(void) {
    uint8_t data[] = "123456789";
    uint32_t expected_crc = 0xCBF43926;
    uint32_t actual_crc = pd_crc32(data, sizeof(data) - 1);
    TEST_ASSERT_EQUAL_HEX32(expected_crc, actual_crc);
}
