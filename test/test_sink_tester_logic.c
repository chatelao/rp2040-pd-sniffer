#include "unity.h"
#include "sink_tester_logic.h"
#include "test_firmware_common.h"

void test_sink_tester_process_packet_valid_source_cap(void) {
    pd_packet_t source_cap_packet = {
        .header = pd_header_build(1, 1, 1, 0, 1, 0),
        .num_data_objects = 1,
        .data = {SOURCE_CAP_PDO},
        .valid = true
    };
    pd_packet_t response_packet;
    TEST_ASSERT_TRUE(sink_tester_process_packet(&source_cap_packet, &response_packet));
    TEST_ASSERT_EQUAL(pd_header_build(1, 2, 0, 0, 1, 1), response_packet.header);
    TEST_ASSERT_EQUAL(1, response_packet.num_data_objects);
    TEST_ASSERT_EQUAL((1 << 28) | (150 << 10) | 150, response_packet.data[0]);
}

void test_sink_tester_process_packet_invalid_source_cap(void) {
    pd_packet_t invalid_source_cap_packet = {
        .header = pd_header_build(1, 1, 1, 0, 1, 0),
        .num_data_objects = 1,
        .data = {0}, // Invalid PDO
        .valid = true
    };
    pd_packet_t response_packet;
    TEST_ASSERT_FALSE(sink_tester_process_packet(&invalid_source_cap_packet, &response_packet));
}

void test_sink_tester_process_packet_not_source_cap(void) {
    pd_packet_t not_source_cap_packet = {
        .header = 0x1234,
        .num_data_objects = 0,
        .valid = true
    };
    pd_packet_t response_packet;
    TEST_ASSERT_FALSE(sink_tester_process_packet(&not_source_cap_packet, &response_packet));
}

void test_sink_tester_process_packet_invalid(void) {
    pd_packet_t invalid_packet = { .valid = false };
    pd_packet_t response_packet;
    TEST_ASSERT_FALSE(sink_tester_process_packet(&invalid_packet, &response_packet));
}
