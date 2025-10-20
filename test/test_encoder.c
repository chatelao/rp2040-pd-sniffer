#include "unity.h"
#include "pd_library.h"
#include "test_data.h"

void test_encode_goodcrc(void) {
    pd_packet_t packet = {
        .header = 0x0041, // GoodCRC
        .num_data_objects = 0
    };
    uint32_t encoded_data[10];
    size_t encoded_len;

    pd_encode_packet(&packet, encoded_data, &encoded_len);

    TEST_ASSERT_EQUAL(goodcrc_encoded_len, encoded_len);
    TEST_ASSERT_EQUAL_HEX32_ARRAY(goodcrc_encoded, encoded_data, encoded_len);
}

// ... other tests can be added here
