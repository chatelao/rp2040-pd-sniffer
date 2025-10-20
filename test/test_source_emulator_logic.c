#include "unity.h"
#include "source_emulator_logic.h"
#include "test_firmware_common.h"

void test_source_emulator_prepare_packet(void) {
    pd_packet_t packet;
    source_emulator_prepare_packet(&packet);
    TEST_ASSERT_EQUAL(pd_header_build(1, 1, 1, 0, 1, 0), packet.header);
    TEST_ASSERT_EQUAL(1, packet.num_data_objects);
    TEST_ASSERT_EQUAL(SOURCE_CAP_PDO, packet.data[0]);
}
