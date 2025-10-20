#include "pinger_logic.h"
#include "test_firmware_common.h"

bool pinger_process_packet(pd_packet_t* packet) {
    if (packet->valid && (packet->header & 0x7FFF) == PONG_VDM_HEADER) {
        if (packet->num_data_objects > 0 && packet->data[0] == PONG_VDM_DATA[0]) {
            return true; // It's a PONG
        }
    }
    return false;
}
