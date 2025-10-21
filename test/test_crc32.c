#include "unity.h"
#include "pd_library.h"

void test_crc32_goodcrc(void) {
    uint8_t data[] = {0x41, 0x00}; // GoodCRC message
    uint32_t crc = pd_crc32(data, sizeof(data));
    TEST_ASSERT_EQUAL_HEX32(0x8A01C7DE, crc);
}

void test_crc32_source_capabilities(void) {
    uint8_t data[] = {0xA1, 0x01, 0x2C, 0x01, 0x90, 0x09}; // Source_Capabilities message
    uint32_t crc = pd_crc32(data, sizeof(data));
    TEST_ASSERT_EQUAL_HEX32(0x2B1AE4AF, crc);
}
