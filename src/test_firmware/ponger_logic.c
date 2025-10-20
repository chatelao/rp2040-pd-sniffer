#include "ponger_logic.h"
#include "test_firmware_common.h"

bool ponger_process_packet(pd_packet_t* received_packet, pd_packet_t* response_packet) {
    if (received_packet->valid && (received_packet->header & 0x7FFF) == PING_VDM_HEADER) {
        if (received_packet->num_data_objects > 0 && received_packet->data[0] == PING_VDM_DATA[0]) {
            response_packet->header = PONG_VDM_HEADER;
            response_packet->num_data_objects = 1;
            response_packet->data[0] = PONG_VDM_DATA[0];
            return true; // Response packet created
        }
    }
    return false;
}
