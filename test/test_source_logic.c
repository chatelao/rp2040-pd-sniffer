#include "unity.h"
#include "pd_source.h"

void test_vdm_unlock(void) {
    source_init();
    source_tick(NULL, 0);

    pd_packet_t packet;
    packet.header = pd_header_build(1, 0xF, false, false, 2, 0);
    packet.num_data_objects = 1;
    packet.data[0] = 0xFF00; // VDM_PROPRIETARY_UNLOCK_SEQ
    packet.valid = true;

    source_handle_packet(NULL, 0, &packet);

    // After the VDM unlock, the source should advertise two PDOs
    // This is an indirect way to test the internal state.
    // A better approach would be to have a getter for the state,
    // but this is sufficient for now.

    // To properly test this, we would need to capture the transmitted packet.
    // For now, we will just check if the vdm_unlocked flag is set.
    // This requires exposing the flag for testing.
    extern bool vdm_unlocked;
    TEST_ASSERT_TRUE(vdm_unlocked);
}
