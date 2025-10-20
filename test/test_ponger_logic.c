#include "unity.h"
#include "ponger_logic.h"
#include "test_firmware_common.h"

void test_ponger_process_packet_ping(void) {
    pd_packet_t ping_packet = {
        .header = PING_VDM_HEADER,
        .num_data_objects = 1,
        .data = {PING_VDM_DATA[0]},
        .valid = true
    };
    pd_packet_t response_packet;
    TEST_ASSERT_TRUE(ponger_process_packet(&ping_packet, &response_packet));
    TEST_ASSERT_EQUAL(PONG_VDM_HEADER, response_packet.header);
    TEST_ASSERT_EQUAL(1, response_packet.num_data_objects);
    TEST_ASSERT_EQUAL(PONG_VDM_DATA[0], response_packet.data[0]);
}

void test_ponger_process_packet_not_ping(void) {
    pd_packet_t not_ping_packet = {
        .header = 0x1234,
        .num_data_objects = 0,
        .valid = true
    };
    pd_packet_t response_packet;
    TEST_ASSERT_FALSE(ponger_process_packet(&not_ping_packet, &response_packet));
}

void test_ponger_process_packet_invalid(void) {
    pd_packet_t invalid_packet = { .valid = false };
    pd_packet_t response_packet;
    TEST_ASSERT_FALSE(ponger_process_packet(&invalid_packet, &response_packet));
}
