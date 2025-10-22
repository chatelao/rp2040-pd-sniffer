#include "pd_source.h"
#include "hal/hal.h"
#include <string.h>

#ifndef NATIVE_BUILD
#include "pico/time.h"
#endif

static source_state_t state;
static uint32_t message_id_counter;
static uint64_t state_timer;
static unsigned int tx_sm;

void source_init(unsigned int sm) {
    state = SOURCE_STATE_INIT;
    message_id_counter = 0;
    state_timer = 0;
    tx_sm = sm;
}

static void send_good_crc(int message_id) {
    pd_packet_t packet;
    packet.header = pd_header_build(0, 0x1, true, false, 2, message_id);
    packet.num_data_objects = 0;
    pd_transmit_packet(tx_sm, &packet);
}

static void send_source_capabilities(void) {
    pd_packet_t packet;
    packet.header = pd_header_build(1, 0x1, true, false, 2, message_id_counter);
    packet.num_data_objects = 1;
    packet.data[0] = (0 << 30) | (0 << 28) | (0 << 26) | (0 << 25) | (0 << 24) | (300 << 10) | (300 << 0);
    pd_transmit_packet(tx_sm, &packet);
    message_id_counter = (message_id_counter + 1) % 8;
}

static void send_accept(int message_id) {
    pd_packet_t packet;
    packet.header = pd_header_build(0, 0x2, true, false, 2, message_id);
    packet.num_data_objects = 0;
    pd_transmit_packet(tx_sm, &packet);
}

static void send_ps_rdy(int message_id) {
    pd_packet_t packet;
    packet.header = pd_header_build(0, 0x6, true, false, 2, message_id);
    packet.num_data_objects = 0;
    pd_transmit_packet(tx_sm, &packet);
}

void source_tick(void) {
    if (state == SOURCE_STATE_INIT) {
#ifndef NATIVE_BUILD
        if (time_us_64() - state_timer > 100000) {
            send_source_capabilities();
            state_timer = time_us_64();
        }
#endif
    }
}

void source_handle_packet(pd_packet_t* packet) {
    uint16_t message_type = packet->header & 0x1F;
    uint8_t message_id = (packet->header >> 9) & 0x7;

    if (message_type == 0x2) { // Request
        send_good_crc(message_id);
        send_accept(message_id);
        send_ps_rdy(message_id);
        state = SOURCE_STATE_READY;
    }
}
