#include "unity.h"
#include "pd_library.h"
#include "test_data.h"

pd_packet_t decoded_packet;
bool packet_received;

void packet_callback(pd_packet_t* packet) {
    decoded_packet = *packet;
    packet_received = true;
}

void setUp(void) {
    packet_received = false;
    bmc_decoder_reset();
}

void test_decode_goodcrc(void) {
    for (int i = 0; i < goodcrc_bmc_len; ++i) {
        bmc_decoder_feed(goodcrc_bmc[i], packet_callback);
    }
    TEST_ASSERT_TRUE(packet_received);
    TEST_ASSERT_TRUE(decoded_packet.valid);
    TEST_ASSERT_EQUAL(0, decoded_packet.num_data_objects);
}

// ... more tests
