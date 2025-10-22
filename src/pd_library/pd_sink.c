#include "pd_sink.h"
#include "hal/hal.h"
#include <string.h>

#ifndef NATIVE_BUILD
#include "pico/time.h"
#endif

void sink_init(pd_sink_t* sink, unsigned int sm) {
    sink->state = SINK_STATE_INIT;
    sink->message_id_counter = 0;
    sink->state_timer = 0;
    sink->tx_sm = sm;
}

static void send_good_crc(pd_sink_t* sink, int message_id) {
    pd_packet_t packet;
    packet.header = pd_header_build(0, 0x1, false, false, 2, message_id);
    packet.num_data_objects = 0;
    pd_transmit_packet(sink->tx_sm, &packet);
}

static void send_request(pd_sink_t* sink, int object_position) {
    pd_packet_t packet;
    packet.header = pd_header_build(1, 0x2, false, false, 2, sink->message_id_counter);
    packet.num_data_objects = 1;

    // Fixed 15V, 3A request
    packet.data[0] = (object_position << 28) | (1 << 27) | (0 << 25) | (0 << 24) | (300 << 10) | (300 << 0);
    pd_transmit_packet(sink->tx_sm, &packet);

    sink->message_id_counter = (sink->message_id_counter + 1) % 8;
    sink->state = SINK_STATE_REQUESTING;
#ifndef NATIVE_BUILD
    sink->state_timer = time_us_64();
#endif
}

void sink_tick(pd_sink_t* sink) {
    if (sink->state == SINK_STATE_INIT) {
        sink->state = SINK_STATE_READY;
    }

#ifndef NATIVE_BUILD
    if (sink->state == SINK_STATE_REQUESTING) {
        if (time_us_64() - sink->state_timer > 500000) {
            sink->state = SINK_STATE_READY;
        }
    }
#endif
}

void sink_handle_packet(pd_sink_t* sink, pd_packet_t* packet) {
    uint16_t message_type = packet->header & 0x1F;
    uint8_t message_id = (packet->header >> 9) & 0x7;

#ifndef NATIVE_BUILD
    if (sink->state == SINK_STATE_READY) {
        if (message_type == 0x1) { // Source_Capabilities
            send_good_crc(sink, message_id);
            for (int i = 0; i < packet->num_data_objects; ++i) {
                uint32_t pdo = packet->data[i];
                if ((pdo >> 30) == 0) { // Fixed supply
                    uint32_t voltage = (pdo >> 10) & 0x3FF;
                    uint32_t current = pdo & 0x3FF;
                    if (voltage == 150 && current >= 30) { // 15V, 3A
                        send_request(sink, i + 1);
                        break;
                    }
                }
            }
        }
    } else if (sink->state == SINK_STATE_REQUESTING) {
        if (message_type == 0x2) { // Accept
            send_good_crc(sink, message_id);
            sink->state = SINK_STATE_READY;
        } else if (message_type == 0x3) { // Reject
            send_good_crc(sink, message_id);
            sink->state = SINK_STATE_READY;
        }
    }
#endif
}
