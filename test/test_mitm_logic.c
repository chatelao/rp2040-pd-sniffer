#include "unity.h"
#include "pd_mitm.h"

void test_mitm_negotiation(void) {
    mitm_init();

    // --- Step 1: Source advertises capabilities ---
    pd_packet_t source_caps;
    source_caps.header = pd_header_build(1, 0x1, true, false, 2, 0);
    source_caps.num_data_objects = 1;
    source_caps.data[0] = (0 << 30) | (1 << 28) | (0 << 26) | (100 << 10) | 300; // 5V, 3A
    source_caps.valid = true;
    mitm_handle_packet_from_source(NULL, 0, &source_caps);

    // --- Step 2: MitM requests power ---
    mitm_tick(NULL, 0, NULL, 0);

    // --- Step 3: Source accepts request ---
    pd_packet_t accept;
    accept.header = pd_header_build(0, 0x2, true, false, 2, 0);
    accept.num_data_objects = 0;
    accept.valid = true;
    mitm_handle_packet_from_source(NULL, 0, &accept);

    // --- Step 4: MitM advertises its own capabilities ---
    mitm_tick(NULL, 0, NULL, 0);

    // To properly test this, we would need to capture the transmitted packet.
    // For now, we will just check the internal state.
    extern mitm_state_t state;
    TEST_ASSERT_EQUAL(MITM_STATE_READY_TO_SINK, state);
}
