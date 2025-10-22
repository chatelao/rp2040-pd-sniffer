#include "pd_sink.h"
#include "pd_common.h"
#include <string.h>

#ifndef NATIVE_BUILD
#include "pico/time.h"
#endif

static sink_state_t state;
static uint32_t message_id_counter;
static uint64_t state_timer;

void sink_init(void) {
    state = SINK_STATE_INIT;
    message_id_counter = 0;
    state_timer = 0;
}

static void send_good_crc(PIO pio, uint sm, int message_id) {
    pd_packet_t packet;
    packet.header = pd_header_build(0, 0x1, false, false, 2, message_id);
    packet.num_data_objects = 0;
#ifndef NATIVE_BUILD
    pd_transmit_packet(pio, sm, &packet);
#endif
}

static void send_request(PIO pio, uint sm, int object_position) {
    pd_packet_t packet;
    packet.header = pd_header_build(1, 0x2, false, false, 2, message_id_counter);
    packet.num_data_objects = 1;

    // Fixed 15V, 3A request
    packet.data[0] = (object_position << 28) | (1 << 27) | (0 << 25) | (0 << 24) | (300 << 10) | (300 << 0);
#ifndef NATIVE_BUILD
    pd_transmit_packet(pio, sm, &packet);
#endif

    message_id_counter = (message_id_counter + 1) % 8;
    state = SINK_STATE_REQUESTING;
    state_timer = time_us_64();
}

void sink_tick(void) {
    if (state == SINK_STATE_INIT) {
        state = SINK_STATE_READY;
    }

    if (state == SINK_STATE_REQUESTING) {
        if (time_us_64() - state_timer > 500000) {
            state = SINK_STATE_READY;
        }
    }
}

void sink_handle_packet(PIO pio, uint sm, pd_packet_t* packet) {
    uint16_t message_type = packet->header & 0x1F;
    uint8_t message_id = (packet->header >> 9) & 0x7;

    if (state == SINK_STATE_READY) {
        if (message_type == 0x1) { // Source_Capabilities
            send_good_crc(pio, sm, message_id);
            for (int i = 0; i < packet->num_data_objects; ++i) {
                uint32_t pdo = packet->data[i];
                if ((pdo >> 30) == 0) { // Fixed supply
                    uint32_t voltage = (pdo >> 10) & 0x3FF;
                    uint32_t current = pdo & 0x3FF;
                    if (voltage == 150 && current >= 30) { // 15V, 3A
                        send_request(pio, sm, i + 1);
                        break;
                    }
                }
            }
        }
    } else if (state == SINK_STATE_REQUESTING) {
        if (message_type == 0x2) { // Accept
            send_good_crc(pio, sm, message_id);
            state = SINK_STATE_READY;
        } else if (message_type == 0x3) { // Reject
            send_good_crc(pio, sm, message_id);
            state = SINK_STATE_READY;
        }
    }
}
