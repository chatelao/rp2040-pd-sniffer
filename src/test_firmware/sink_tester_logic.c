#include "sink_tester_logic.h"
#include "test_firmware_common.h"

bool sink_tester_process_packet(pd_packet_t* received_packet, pd_packet_t* response_packet) {
    if (received_packet->valid && received_packet->num_data_objects > 0 && (received_packet->header & 0x1F) == 1) { // Source_Capabilities
        uint32_t pdo = received_packet->data[0];
        if (pdo == SOURCE_CAP_PDO) {
            response_packet->header = pd_header_build(1, 2, 0, 0, 1, 1); // Request
            response_packet->num_data_objects = 1;
            response_packet->data[0] = (1 << 28) | (150 << 10) | 150; // 3A, 3A
            return true; // Response packet created
        }
    }
    return false;
}
