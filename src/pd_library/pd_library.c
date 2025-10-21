#include "pd_library.h"
#include <string.h>

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

#define CRC_POLY 0x04C11DB7

uint32_t pd_crc32(const uint8_t *data, size_t len) {
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= (uint32_t)data[i] << 24;
        for (int j = 0; j < 8; j++) {
            if (crc & 0x80000000) {
                crc = (crc << 1) ^ CRC_POLY;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc ^ 0xFFFFFFFF;
}

#define K_CODE_SYNC1 0b11000
#define K_CODE_SYNC2 0b10001
#define K_CODE_EOP   0b01101

void pd_decode_packet(uint32_t* captured_data, uint32_t data_len, pd_packet_t* packet) {
    packet->valid = false;

    enum { STATE_SOP, STATE_PACKET } decoder_state = STATE_SOP;

    uint64_t shift_reg = 0;
    int bit_count = 0;

    pd_packet_t current_packet;
    uint8_t packet_buffer[34];
    int quintet_count = 0;
    enum { PKT_STATE_HEADER, PKT_STATE_DATA, PKT_STATE_CRC } packet_state = PKT_STATE_HEADER;

    for (uint32_t i = 0; i < data_len; i++) {
        uint32_t word = captured_data[i];
        for (int j = 0; j < 32; j++) {
            int bit = (word >> (31 - j)) & 1;
            shift_reg = (shift_reg << 1) | bit;

            switch (decoder_state) {
                case STATE_SOP:
                    if ((shift_reg & 0xFFFFF) == ((K_CODE_SYNC1 << 15) | (K_CODE_SYNC1 << 10) | (K_CODE_SYNC2 << 5) | K_CODE_SYNC2)) {
                        decoder_state = STATE_PACKET;
                        packet_state = PKT_STATE_HEADER;
                        bit_count = 0;
                        quintet_count = 0;
                        memset(&current_packet, 0, sizeof(current_packet));
                        memset(packet_buffer, 0, sizeof(packet_buffer));
                    }
                    break;

                case STATE_PACKET:
                    bit_count++;
                    if (bit_count == 5) {
                        bit_count = 0;
                        uint8_t symbol = shift_reg & 0x1F;

                        if (symbol == K_CODE_EOP) {
                            int expected_quintets = 4 + current_packet.num_data_objects * 8 + 8;
                            if (quintet_count == expected_quintets) {
                                // Finalize packet
                                memcpy(&current_packet.header, packet_buffer, 2);
                                memcpy(current_packet.data, packet_buffer + 2, current_packet.num_data_objects * 4);
                                memcpy(&current_packet.crc, packet_buffer + 2 + current_packet.num_data_objects * 4, 4);

                                uint8_t crc_data[2 + MAX_DATA_OBJECTS * 4];
                                memcpy(crc_data, &current_packet.header, 2);
                                if (current_packet.num_data_objects > 0) {
                                   memcpy(crc_data + 2, current_packet.data, current_packet.num_data_objects * 4);
                                }
                                uint32_t calculated_crc = pd_crc32(crc_data, 2 + current_packet.num_data_objects * 4);

                                if (calculated_crc == current_packet.crc) {
                                    *packet = current_packet;
                                    packet->valid = true;
                                    return;
                                }
                            }
                            decoder_state = STATE_SOP;
                        } else {
                            uint8_t nybble = fiveb_to_fourb[symbol];
                            int byte_pos = quintet_count / 2;
                            if (quintet_count < sizeof(packet_buffer) * 2) {
                                if (quintet_count % 2 == 0) packet_buffer[byte_pos] = nybble;
                                else packet_buffer[byte_pos] |= (nybble << 4);
                            }
                            quintet_count++;

                            if (packet_state == PKT_STATE_HEADER && quintet_count == 4) {
                                memcpy(&current_packet.header, packet_buffer, 2);
                                current_packet.num_data_objects = (current_packet.header >> 12) & 0x7;
                                if (current_packet.num_data_objects > 7) current_packet.num_data_objects = 0;
                                packet_state = (current_packet.num_data_objects == 0) ? PKT_STATE_CRC : PKT_STATE_DATA;
                            } else if (packet_state == PKT_STATE_DATA && quintet_count == 4 + current_packet.num_data_objects * 8) {
                                packet_state = PKT_STATE_CRC;
                            }
                        }
                    }
                    break;
            }
        }
    }
}


static void append_4b5b(uint32_t* stream, int* stream_len, uint16_t data) {
    stream[*stream_len] = fourb_to_fiveb[(data >> 0) & 0xF];
    stream[*stream_len + 1] = fourb_to_fiveb[(data >> 4) & 0xF];
    stream[*stream_len + 2] = fourb_to_fiveb[(data >> 8) & 0xF];
    stream[*stream_len + 3] = fourb_to_fiveb[(data >> 12) & 0xF];
    *stream_len += 4;
}

void pd_encode_packet(pd_packet_t* packet, uint32_t* encoded_data, size_t* encoded_len) {
    uint32_t stream[64];
    int stream_len = 0;

    // Preamble and SOP
    stream[stream_len++] = 0b10101010;
    stream[stream_len++] = 0b10101010;
    stream[stream_len++] = 0b10101010;
    stream[stream_len++] = 0b10101010;
    stream[stream_len++] = 0b10101010;
    stream[stream_len++] = 0b10101010;
    stream[stream_len++] = 0b10101011;

    append_4b5b(stream, &stream_len, packet->header);
    for (int i = 0; i < packet->num_data_objects; ++i) {
        append_4b5b(stream, &stream_len, (packet->data[i] >> 0) & 0xFFFF);
        append_4b5b(stream, &stream_len, (packet->data[i] >> 16) & 0xFFFF);
    }

    uint8_t crc_data[2 + packet->num_data_objects * 4];
    memcpy(crc_data, &packet->header, 2);
    memcpy(crc_data + 2, packet->data, packet->num_data_objects * 4);
    uint32_t crc = pd_crc32(crc_data, sizeof(crc_data));

    append_4b5b(stream, &stream_len, (crc >> 0) & 0xFFFF);
    append_4b5b(stream, &stream_len, (crc >> 16) & 0xFFFF);

    stream[stream_len++] = 0b01110; // EOP

    // BMC encode
    int bit_count = 0;
    uint32_t bmc_stream = 0;
    for (int i = 0; i < stream_len; ++i) {
        for (int j = 0; j < 5; ++j) {
            uint32_t bit = (stream[i] >> j) & 1;
            if (bit_count > 0 && bit != ((bmc_stream >> (bit_count - 1)) & 1)) {
                bmc_stream |= (1 << bit_count); // Add transition
                bit_count++;
            }
            bmc_stream |= (bit << bit_count);
            bit_count++;
        }
    }

    if (bit_count > 32) {
        encoded_data[0] = bmc_stream;
        encoded_data[1] = bmc_stream >> 32;
        *encoded_len = 2;
    } else {
        encoded_data[0] = bmc_stream;
        *encoded_len = 1;
    }
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

void pd_receiver_init(PIO pio, uint sm, uint pin) {
    uint offset = pio_add_program(pio, &pd_receiver_program);
    pio_sm_config c = pd_receiver_program_get_default_config(offset);
    sm_config_set_in_pins(&c, pin);
    sm_config_set_in_shift(&c, false, true, 32);
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_RX);
    // The receiver runs at 10 MHz, so we need a divider of 12.5
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
