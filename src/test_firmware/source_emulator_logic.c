#include "source_emulator_logic.h"
#include "test_firmware_common.h"

void source_emulator_prepare_packet(pd_packet_t* packet) {
    packet->header = pd_header_build(1, 1, 1, 0, 1, 0); // Source_Capabilities
    packet->num_data_objects = 1;
    packet->data[0] = SOURCE_CAP_PDO;
}
