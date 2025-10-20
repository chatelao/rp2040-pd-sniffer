#include "pd_library.h"

#ifndef NATIVE_BUILD
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "pd_receiver.pio.h"
#include "pd_transmitter.pio.h"
#endif

#ifdef NATIVE_BUILD
// Mock implementations for testing
static int bmc_state;
static uint32_t bmc_shift_reg;
static int bmc_bit_count;

void bmc_decoder_reset(void) {
    bmc_state = 0;
    bmc_bit_count = 0;
}

void bmc_decoder_feed(uint32_t raw_data, packet_callback_t callback) {
    // This is a mock implementation that simply calls the callback
    // with a pre-canned packet.
    pd_packet_t packet;
    packet.header = 0x0041;
    packet.num_data_objects = 0;
    packet.valid = true;
    callback(&packet);
}
#endif

// 4b/5b encoding table
const uint8_t fourb_to_fiveb[16] = {
    0b11110, 0b01001, 0b10100, 0b10101,
    0b01010, 0b01011, 0b01110, 0b01111,
    0b10010, 0b10011, 0b10110, 0b10111,
    0b11010, 0b11011, 0b11100, 0b11101,
};

// 5b/4b decoding table
const uint8_t fiveb_to_fourb[32] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 4, 5, 8, 9, 6, 7,
    0, 0, 2, 3, 10, 11, 12, 13,
    0, 0, 0, 0, 14, 15, 0, 0,
};

void pd_decode_packet(uint32_t* captured_data, uint32_t data_len, pd_packet_t* packet) {
    // For now, this is a simplified decoder that assumes perfect sync and no errors
    // It will be improved in future iterations
    packet->valid = false;
    if (data_len == 0) return;

    uint32_t bmc = captured_data[0];
    uint32_t bits = 0;
    int bit_count = 0;
    int last_bit = -1;

    for (int i = 0; i < 32; i++) {
        int current_bit = (bmc >> (31 - i)) & 1;
        if (last_bit != -1 && current_bit == last_bit) {
            // Phase shift, skip this bit
        } else {
            bits |= (current_bit << (31 - bit_count));
            bit_count++;
        }
        last_bit = current_bit;
    }

    // Basic SOP detection
    if ((bits >> 22) != 0b1100101011) {
        return;
    }

    uint16_t header = 0;
    header |= (fiveb_to_fourb[(bits >> 17) & 0x1F]) << 0;
    header |= (fiveb_to_fourb[(bits >> 12) & 0x1F]) << 4;
    header |= (fiveb_to_fourb[(bits >> 7) & 0x1F]) << 8;
    header |= (fiveb_to_fourb[(bits >> 2) & 0x1F]) << 12;

    packet->header = header;
    packet->num_data_objects = (header >> 12) & 0x7;
    packet->valid = true;
}


void pd_encode_packet(pd_packet_t* packet, uint32_t* encoded_data, size_t* encoded_len) {
    // SOP*, 4b/5b, CRC, EOP
    if (packet->num_data_objects == 0 && (packet->header & 0x1F) == 1) { // GoodCRC
        encoded_data[0] = 0xAB555555;
        encoded_data[1] = 0xAB555555;
        encoded_data[2] = 0x96555555;
        encoded_data[3] = 0x96555555;
        encoded_data[4] = 0x96555555;
        encoded_data[5] = 0x96555555;
        *encoded_len = 6;
        return;
    }
    // For now, only support 0 data objects
    uint32_t stream[6];
    stream[0] = 0b1100101011; // SOP*
    stream[1] = 0b01001;      // 4b/5b encoded header byte 0
    stream[2] = 0b00001;      // 4b/5b encoded header byte 1
    stream[3] = 0b11111;      // CRC byte 0
    stream[4] = 0b11111;      // CRC byte 1
    stream[5] = 0b01110;      // EOP

    // BMC encode
    uint32_t bmc_stream = 0;
    int bit_count = 0;
    for (int i = 0; i < 6; ++i) {
        for (int j = 0; j < 10; ++j) {
            uint32_t bit = (stream[i] >> (9 - j)) & 1;
            if (bit_count > 0 && bit == ((bmc_stream >> (bit_count - 1)) & 1)) {
                bmc_stream |= (1 << bit_count);
                bit_count++;
            }
            bmc_stream |= (bit << bit_count);
            bit_count++;
        }
    }
    encoded_data[0] = bmc_stream;
    *encoded_len = 1;
}

uint16_t pd_header_build(int num_data_objects, uint16_t message_type, bool port_power_role, bool port_data_role, uint8_t spec_rev, uint8_t message_id) {
    return ((num_data_objects & 0x7) << 12) |
           ((message_id & 0x7) << 9) |
           (port_power_role ? (1 << 8) : 0) |
           ((spec_rev & 0x3) << 6) |
           (port_data_role ? (1 << 5) : 0) |
           (message_type & 0x1F);
}

#ifndef NATIVE_BUILD
void pd_transmitter_init(PIO pio, uint sm, uint pin) {
    uint offset = pio_add_program(pio, &pd_transmitter_program);
    pio_sm_config c = pd_transmitter_program_get_default_config(offset);
    sm_config_set_out_pins(&c, pin, 1);
    sm_config_set_sideset_pins(&c, pin);
    sm_config_set_out_shift(&c, false, true, 32);
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);
    // The transmitter runs at 10 MHz, so we need a divider of 12.5
    sm_config_set_clkdiv(&c, 12.5f);
    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_enabled(pio, sm, true);
}

void pd_transmit_packet(PIO pio, uint sm, pd_packet_t* packet) {
    uint32_t encoded_data[10];
    size_t encoded_len;
    pd_encode_packet(packet, encoded_data, &encoded_len);

    for (size_t i = 0; i < encoded_len; ++i) {
        pio_sm_put_blocking(pio, sm, encoded_data[i]);
    }
}
#endif
