#include "unity.h"
#include "pinger_logic.h"
#include "test_firmware_common.h"

void test_pinger_process_packet_pong(void) {
    pd_packet_t pong_packet = {
        .header = PONG_VDM_HEADER,
        .num_data_objects = 1,
        .data = {PONG_VDM_DATA[0]},
        .valid = true
    };
    TEST_ASSERT_TRUE(pinger_process_packet(&pong_packet));
}

void test_pinger_process_packet_not_pong(void) {
    pd_packet_t not_pong_packet = {
        .header = 0x1234,
        .num_data_objects = 0,
        .valid = true
    };
    TEST_ASSERT_FALSE(pinger_process_packet(&not_pong_packet));
}

void test_pinger_process_packet_invalid(void) {
    pd_packet_t invalid_packet = { .valid = false };
    TEST_ASSERT_FALSE(pinger_process_packet(&invalid_packet));
}
