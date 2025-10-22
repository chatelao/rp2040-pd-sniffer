#include "unity.h"
#include "pd_library.h"

void test_pd_build_vdm_header(void) {
    uint32_t header = pd_build_vdm_header(0x1234, true, 1, 0x5);
    TEST_ASSERT_EQUAL_HEX32(0x12348105, header);
}

// Test with 1 data object
void test_pd_build_request_header(void) {
    // num_data_objects=1, message_type=2 (Request), power_role=0 (Sink), data_role=0 (UFP), spec_rev=1 (2.0), msg_id=5
    uint16_t header = pd_header_build(1, 2, 0, 0, 1, 5);
    // Expected: 0b0_001_101_0_01_0_00010 = 0x1A42
    TEST_ASSERT_EQUAL_HEX16(0x1A42, header);
}

// Test with 0 data objects
void test_pd_build_request_header_no_data(void) {
    // num_data_objects=0, message_type=2 (Request), power_role=0 (Sink), data_role=0 (UFP), spec_rev=1 (2.0), msg_id=5
    uint16_t header = pd_header_build(0, 2, 0, 0, 1, 5);
    // Expected: 0b0_000_101_0_01_0_00010 = 0x0A42
    TEST_ASSERT_EQUAL_HEX16(0x0A42, header);
}
