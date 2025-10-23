#include "unity.h"
#include "pd_sink.h"
#include "pd_library.h"

// Mock HAL functions
static pd_packet_t last_transmitted_packet;
static bool packet_transmitted;
static uint64_t current_time;

void pd_transmit_packet(unsigned int sm, const pd_packet_t* packet) {
    last_transmitted_packet = *packet;
    packet_transmitted = true;
}

uint64_t time_us_64(void) {
    return current_time;
}

void test_sink_chooses_pdo_with_lowest_sufficient_current_at_same_voltage(void) {
    packet_transmitted = false;
    current_time = 0;
    pd_sink_t sink;
    sink_init(&sink, 0);
    sink_tick(&sink);
    sink_request_power(&sink, 12000, 1000); // Request 12V, 1A

    pd_packet_t source_caps;
    source_caps.header = pd_header_build(2, 1, false, false, 2, 0);
    source_caps.num_data_objects = 2;
    source_caps.data[0] = (240 << 10) | 300; // 12V, 3A
    source_caps.data[1] = (240 << 10) | 100; // 12V, 1A

    sink_handle_packet(&sink, &source_caps);

    // Verify that the sink requested the correct PDO (the second one, with the lower current)
    TEST_ASSERT_TRUE(packet_transmitted);
    uint8_t requested_object_position = (last_transmitted_packet.data[0] >> 28) & 0x7;
    TEST_ASSERT_EQUAL(2, requested_object_position);
}
