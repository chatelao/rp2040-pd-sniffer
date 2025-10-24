#include "pd_library.h"
#include "hal/hal.h"
#include <string.h>

#ifndef NATIVE_BUILD
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "pd_receiver.pio.h"
#include "pd_transmitter.pio.h"
#else
#include <stdio.h>
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

/**
 * @brief 4b/5b encoding table.
 */
const uint8_t fourb_to_fiveb[16] = {
    0b11110, 0b01001, 0b10100, 0b10101,
    0b01010, 0b01011, 0b01110, 0b01111,
    0b10010, 0b10011, 0b10110, 0b10111,
    0b11010, 0b11011, 0b11100, 0b11101,
};

/**
 * @brief CRC32 table.
 */
static uint32_t crc_table[256];
static bool crc_table_generated = false;

/**
 * @brief Generates the CRC32 table.
 */
static void generate_crc_table(void) {
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t c = i;
        for (int j = 0; j < 8; j++) {
            if (c & 1) {
                c = 0xEDB88320L ^ (c >> 1);
            } else {
                c = c >> 1;
            }
        }
        crc_table[i] = c;
    }
}

uint32_t pd_crc32(const uint8_t *data, size_t len) {
    if (!crc_table_generated) {
        generate_crc_table();
        crc_table_generated = true;
    }

    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < len; i++) {
        crc = (crc >> 8) ^ crc_table[(crc ^ data[i]) & 0xFF];
    }
    return ~crc;
}

#define K_CODE_SYNC1 0b11000
#define K_CODE_SYNC2 0b10001
#define K_CODE_EOP   0b01101

/**
 * @brief Appends a 16-bit value to a 4b/5b encoded stream.
 * @param stream The stream to append to.
 * @param stream_len A pointer to the length of the stream.
 * @param data The data to append.
 */
static void append_4b5b(uint32_t* stream, int* stream_len, uint16_t data) {
    stream[*stream_len] = fourb_to_fiveb[(data >> 0) & 0xF];
    stream[*stream_len + 1] = fourb_to_fiveb[(data >> 4) & 0xF];
    stream[*stream_len + 2] = fourb_to_fiveb[(data >> 8) & 0xF];
    stream[*stream_len + 3] = fourb_to_fiveb[(data >> 12) & 0xF];
    *stream_len += 4;
}

/**
 * @brief Appends a 32-bit value to a 4b/5b encoded stream.
 * @param stream The stream to append to.
 * @param stream_len A pointer to the length of the stream.
 * @param data The data to append.
 */
static void append_4b5b_u32(uint32_t* stream, int* stream_len, uint32_t data) {
    append_4b5b(stream, stream_len, (data >> 0) & 0xFFFF);
    append_4b5b(stream, stream_len, (data >> 16) & 0xFFFF);
}

void pd_encode_packet(const pd_packet_t* packet, uint32_t* encoded_data, size_t* encoded_len) {
    uint32_t stream[64];
    int stream_len = 0;

    // SOP
    stream[stream_len++] = K_CODE_SYNC1;
    stream[stream_len++] = K_CODE_SYNC1;
    stream[stream_len++] = K_CODE_SYNC2;
    stream[stream_len++] = K_CODE_SYNC2;

    append_4b5b(stream, &stream_len, packet->header);
    for (int i = 0; i < packet->num_data_objects; ++i) {
        append_4b5b_u32(stream, &stream_len, packet->data[i]);
    }

    uint8_t crc_data[2 + packet->num_data_objects * 4];
    memcpy(crc_data, &packet->header, 2);
    if (packet->num_data_objects > 0) {
        memcpy(crc_data + 2, packet->data, packet->num_data_objects * 4);
    }
    uint32_t crc = pd_crc32(crc_data, 2 + packet->num_data_objects * 4);
    append_4b5b_u32(stream, &stream_len, crc);

    stream[stream_len++] = 0b01110; // EOP

    // BMC encode
    *encoded_len = 0;
    int bit_count = 0;
    uint64_t bmc_stream = 0;
    int last_bit = 1;
    uint64_t preamble = 0xAAAAAAAAAAAAAAAB;

    for (int i = 0; i < 64; ++i) {
        uint32_t bit = (preamble >> i) & 1;
        if (bit == last_bit) {
            bmc_stream |= (1ULL << bit_count);
            bit_count++;
        }
        bmc_stream |= ((uint64_t)bit << bit_count);
        bit_count++;
        last_bit = bit;
        if (bit_count >= 32) {
            encoded_data[*encoded_len] = bmc_stream;
            (*encoded_len)++;
            bmc_stream >>= 32;
            bit_count -= 32;
        }
    }

    for (int i = 0; i < stream_len; ++i) {
        for (int j = 4; j >= 0; --j) {
            uint32_t bit = (stream[i] >> j) & 1;
            if (bit == last_bit) {
                bmc_stream |= (1ULL << bit_count);
                bit_count++;
            }
            bmc_stream |= ((uint64_t)bit << bit_count);
            bit_count++;
            last_bit = bit;

            if (bit_count >= 32) {
                encoded_data[*encoded_len] = bmc_stream;
                (*encoded_len)++;
                bmc_stream >>= 32;
                bit_count -= 32;
            }
        }
    }
    if (bit_count > 0) {
        encoded_data[*encoded_len] = bmc_stream;
        (*encoded_len)++;
    }
}

void __attribute__((weak)) pd_transmit_packet(unsigned int sm, const pd_packet_t* packet) {
    uint32_t encoded_data[10];
    size_t encoded_len;
    pd_encode_packet(packet, encoded_data, &encoded_len);

    for (size_t i = 0; i < encoded_len; ++i) {
        hal_pio_sm_put(sm, encoded_data[i]);
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

uint32_t pd_build_vdm_header(uint16_t vendor_id, bool structured, uint8_t command_type, uint8_t command) {
    uint32_t vdm_header = 0;
    vdm_header |= (vendor_id & 0xFFFF) << 16;
    vdm_header |= (structured & 0x1) << 15;
    vdm_header |= (command_type & 0x7) << 8;
    vdm_header |= (command & 0x1F) << 0;
    return vdm_header;
}
