#include "pd_source.h"
#include "pd_common.h"
#include <string.h>

#ifndef NATIVE_BUILD
#include "pico/time.h"
#endif

#define VDM_PROPRIETARY_UNLOCK_SEQ 0xFF00

static source_state_t state;
static uint32_t message_id_counter;
static uint64_t state_timer;
#ifdef NATIVE_BUILD
bool vdm_unlocked = false;
#else
static bool vdm_unlocked = false;
#endif

void send_source_capabilities(PIO pio, uint sm) {
    pd_packet_t packet;
    int num_data_objects = vdm_unlocked ? 2 : 1;
    packet.header = pd_header_build(num_data_objects, 0x1, true, false, 2, message_id_counter);
    packet.num_data_objects = num_data_objects;

    // Standard 5V, 3A profile (5V @ 100mA units, 3A @ 50mA units)
    packet.data[0] = (0 << 30) | (1 << 28) | (0 << 26) | (100 << 10) | 300;

    if (vdm_unlocked) {
        // Proprietary 20V, 5A profile (20V @ 50mV units, 5A @ 10mA units)
        packet.data[1] = (0 << 30) | (1 << 28) | (0 << 26) | (400 << 10) | 500;
    }
#ifndef NATIVE_BUILD
    pd_transmit_packet(pio, sm, &packet);
#endif
    message_id_counter = (message_id_counter + 1) % 8;
}

void source_init(void) {
    state = SOURCE_STATE_INIT;
    message_id_counter = 0;
    state_timer = 0;
}

void source_tick(PIO pio, uint sm) {
    if (state == SOURCE_STATE_INIT) {
        state = SOURCE_STATE_READY;
    }

    if (state == SOURCE_STATE_READY) {
        // Periodically send source capabilities
        if (time_us_64() - state_timer > 500000) {
            send_source_capabilities(pio, sm);
            state_timer = time_us_64();
        }
    }
}

source_state_t source_get_state(void) {
    return state;
}

void source_send_vdm(PIO pio, uint sm) {
    pd_packet_t packet;
    packet.header = pd_header_build(1, 0xF, true, false, 2, message_id_counter);
    packet.num_data_objects = 1;
    packet.data[0] = VDM_PROPRIETARY_UNLOCK_SEQ;
#ifndef NATIVE_BUILD
    pd_transmit_packet(pio, sm, &packet);
#endif
    message_id_counter = (message_id_counter + 1) % 8;
}

static void send_accept(PIO pio, uint sm, int message_id) {
    pd_packet_t packet;
    packet.header = pd_header_build(0, 0x2, true, false, 2, message_id);
    packet.num_data_objects = 0;
#ifndef NATIVE_BUILD
    pd_transmit_packet(pio, sm, &packet);
#endif
}

static void send_reject(PIO pio, uint sm, int message_id) {
    pd_packet_t packet;
    packet.header = pd_header_build(0, 0x3, true, false, 2, message_id);
    packet.num_data_objects = 0;
#ifndef NATIVE_BUILD
    pd_transmit_packet(pio, sm, &packet);
#endif
}

void source_handle_packet(PIO pio, uint sm, pd_packet_t* packet) {
    uint16_t message_type = packet->header & 0x1F;
    uint8_t message_id = (packet->header >> 9) & 0x7;

    if (state == SOURCE_STATE_READY) {
        if (message_type == 0x2) { // Request
            uint32_t rdo = packet->data[0];
            int object_position = (rdo >> 28) & 0x7;
            if (object_position == 1) {
                send_accept(pio, sm, message_id);
                state = SOURCE_STATE_NEGOTIATING;
                state_timer = time_us_64();
            } else if (vdm_unlocked && object_position == 2) {
                send_accept(pio, sm, message_id);
                state = SOURCE_STATE_PROPRIETARY;
                state_timer = time_us_64();
            }
            else {
                send_reject(pio, sm, message_id);
            }
        } else if (message_type == 0xF) { // Vendor_Defined_Message
            uint16_t vdm_header = packet->data[0] & 0xFFFF;
            if (vdm_header == VDM_PROPRIETARY_UNLOCK_SEQ) {
                vdm_unlocked = true;
                send_source_capabilities(pio, sm);
            }
        }
    }
}
