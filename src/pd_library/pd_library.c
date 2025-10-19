#include "pd_library.h"
#include <stdio.h>

// --- Private Constants ---

#define PD_CLK_HZ 300000

// 4b/5b symbol decoding table (maps 5-bit code to 4-bit nibble)
const uint8_t DECODE_4B5B[32] = {
    0xFF, 0x01, 0xFF, 0xFF, 0x0A, 0x0B, 0xFF, 0x07, // 00xxx
    0xFF, 0x09, 0x08, 0xFF, 0x0D, 0x0C, 0x0E, 0x0F, // 01xxx
    0xFF, 0x02, 0x03, 0xFF, 0x04, 0x05, 0x06, 0xFF, // 10xxx
    0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF  // 11xxx
};

// K-Codes for control signals
#define K_SYNC1 0b11000
#define K_SYNC2 0b10001
#define K_RST1  0b00111
#define K_RST2  0b11001
#define K_EOP   0b01101

// --- Private Function Prototypes ---

void process_decoded_bit(bool bit_val, void (*packet_callback)(pd_packet_t*));

// --- Public Function Implementations ---

void bmc_decoder_feed(uint32_t sample_word, void (*packet_callback)(pd_packet_t*)) {
    // State for the BMC decoder
    static uint8_t last_level = 0;
    static uint32_t run_length = 0;
    static uint32_t last_run_length = 0;
    static bool initialized = false;

    // Initialize decoder on first run
    if (!initialized) {
        last_level = sample_word & 1;
        initialized = true;
    }

    for (int i = 0; i < 32; i++) {
        uint8_t current_level = (sample_word >> i) & 1;

        if (current_level != last_level) {
            // Transition detected. Analyze the length of the completed run.
            // T (bit period) = 8 samples.
            // Tolerance window for T/2 (4 samples) and T (8 samples).
            if (run_length >= 6 && run_length <= 10) { // Full-bit run
                // A full-width pulse between transitions means a '0' was sent.
                process_decoded_bit(0, packet_callback);
                last_run_length = 0; // Reset history
            } else if (run_length >= 3 && run_length <= 5) { // Half-bit run
                // A half-width pulse. Could be first or second part of a '1'.
                if (last_run_length >= 3 && last_run_length <= 5) {
                    // Second consecutive half-width pulse. A '1' was sent.
                    process_decoded_bit(1, packet_callback);
                    last_run_length = 0; // Reset history
                } else {
                    // First half-width pulse. Store it and wait for the next.
                    last_run_length = run_length;
                }
            } else {
                // Invalid run length -> noise or sync loss. Reset history.
                last_run_length = 0;
            }

            run_length = 1; // Reset for the next run
            last_level = current_level;
        } else {
            run_length++;
        }
    }
}

// --- Private Function Implementations ---

void process_decoded_bit(bool bit_val, void (*packet_callback)(pd_packet_t*)) {
    // State machine variables
    static decoder_state_t state = STATE_IDLE;
    static uint64_t shift_reg = 0;
    static int idle_bit_count = 0;
    static int packet_bit_count = 0;
    static pd_packet_t current_packet;
    static int data_obj_count = 0;

    // A bit '1' resets our idle detection
    if (bit_val) {
        idle_bit_count = 0;
    } else {
        idle_bit_count++;
    }

    // After ~1.2ms of idle (no transitions), reset to a known state
    if (idle_bit_count > (PD_CLK_HZ / 1000) * 1.2) {
        state = STATE_IDLE;
    }

    // Shift the new bit into our shift register
    shift_reg = (shift_reg << 1) | bit_val;

    if (state != STATE_IDLE) {
        packet_bit_count++;
    }

    switch (state) {
        case STATE_IDLE:
            // Look for the 64-bit preamble
            if ((shift_reg & 0xFFFFFFFFFFFFFFFF) == 0xAAAAAAAAAAAAAAA9) {
                state = STATE_PREAMBLE;
                packet_bit_count = 0;
            }
            break;

        case STATE_PREAMBLE:
            // Preamble is consumed, now look for SOP
            // SOP is composed of 4 K-Codes
            if ((shift_reg & 0xFFFFF) == ((uint32_t)K_SYNC1 << 15 | (uint32_t)K_SYNC1 << 10 | (uint32_t)K_SYNC1 << 5 | K_SYNC2)) {
                state = STATE_SOP;
                packet_bit_count = 0;
                current_packet.valid = false;
                data_obj_count = 0;
            }
            break;

        case STATE_SOP:
            // SOP is consumed, now we're at the start of the message
            state = STATE_HEADER;
            packet_bit_count = 0;
            break;

        case STATE_HEADER:
            // Every 5 bits, decode a symbol
            if (packet_bit_count > 0 && (packet_bit_count % 5 == 0)) {
                uint8_t symbol = (shift_reg >> (packet_bit_count - 5)) & 0x1F;
                uint8_t nibble = DECODE_4B5B[symbol];

                if (nibble == 0xFF) { // Invalid symbol
                    state = STATE_IDLE;
                    break;
                }

                current_packet.header = (current_packet.header << 4) | nibble;

                // After 4 nibbles (20 bits), we have the full header
                if (packet_bit_count == 20) {
                    current_packet.num_data_objs = (current_packet.header >> 12) & 0x7;
                    if (current_packet.num_data_objs == 0) {
                        state = STATE_CRC;
                    } else {
                        state = STATE_DATA;
                    }
                    data_obj_count = 0;
                    packet_bit_count = 0;
                }
            }
            break;

        case STATE_DATA:
            // Similar to header, but for 32-bit data objects
            if (packet_bit_count > 0 && (packet_bit_count % 5 == 0)) {
                uint8_t symbol = (shift_reg >> (packet_bit_count - 5)) & 0x1F;
                uint8_t nibble = DECODE_4B5B[symbol];

                if (nibble == 0xFF) {
                    state = STATE_IDLE;
                    break;
                }

                current_packet.data[data_obj_count] = (current_packet.data[data_obj_count] << 4) | nibble;

                // After 8 nibbles (40 bits), we have a full data object
                if (packet_bit_count == 40) {
                    data_obj_count++;
                    if (data_obj_count == current_packet.num_data_objs) {
                        state = STATE_CRC;
                    }
                    packet_bit_count = 0;
                }
            }
            break;

        case STATE_CRC:
            // Decode the 32-bit CRC
            if (packet_bit_count > 0 && (packet_bit_count % 5 == 0)) {
                uint8_t symbol = (shift_reg >> (packet_bit_count - 5)) & 0x1F;
                uint8_t nibble = DECODE_4B5B[symbol];

                if (nibble == 0xFF) {
                    state = STATE_IDLE;
                    break;
                }

                current_packet.crc = (current_packet.crc << 4) | nibble;

                if (packet_bit_count == 40) {
                    state = STATE_EOP;
                    packet_bit_count = 0;
                }
            }
            break;

        case STATE_EOP:
            // Look for the EOP K-Code
            if ((shift_reg & 0x1F) == K_EOP) {
                current_packet.valid = true;
                if (packet_callback) {
                    packet_callback(&current_packet);
                }
            }
            // In either case, we're done with this packet
            state = STATE_IDLE;
            break;
    }
}


uint16_t pd_build_request_header(uint8_t num_data_objs, uint8_t message_id, uint8_t power_role, uint8_t spec_rev, uint8_t data_role, uint8_t message_type) {
    uint16_t header = 0;
    header |= (num_data_objs & 0x7) << 12;
    header |= (message_id & 0x7) << 9;
    header |= (power_role & 0x1) << 8;
    header |= (spec_rev & 0x3) << 6;
    header |= (data_role & 0x1) << 5;
    header |= (message_type & 0x1F);
    return header;
}

void pd_transmit_packet(PIO pio, uint sm, uint16_t header, const uint32_t *data) {
    // 4b/5b symbol encoding table (maps 4-bit nibble to 5-bit code)
    const uint8_t ENCODE_4B5B[16] = {
        0x1E, 0x09, 0x14, 0x15, 0x0A, 0x0B, 0x0E, 0x0F,
        0x12, 0x13, 0x16, 0x17, 0x1A, 0x1B, 0x1C, 0x1D
    };

    uint32_t bmc_buffer[128]; // Max packet size
    int bmc_bit_count = 0;
    uint8_t last_level = 0;

    // Preamble
    for (int i = 0; i < 64; i++) {
        last_level = (i % 2 == 0) ? 0 : 1;
        bmc_buffer[bmc_bit_count++] = last_level;
        bmc_buffer[bmc_bit_count++] = last_level;
    }

    // SOP
    uint32_t sop = (K_SYNC1 << 15) | (K_SYNC1 << 10) | (K_SYNC1 << 5) | K_SYNC2;
    for (int i = 19; i >= 0; i--) {
        bool bit = (sop >> i) & 1;
        last_level = (last_level == 0) ? 1 : 0;
        bmc_buffer[bmc_bit_count++] = last_level;
        if (bit) {
            last_level = (last_level == 0) ? 1 : 0;
        }
        bmc_buffer[bmc_bit_count++] = last_level;
    }

    // Header and Data
    uint8_t num_data_objs = (header >> 12) & 0x7;
    uint32_t message[8] = {0};
    message[0] = header;
    for (int i = 0; i < num_data_objs; i++) {
        message[i+1] = data[i];
    }

    for (int i = 0; i < 1 + num_data_objs; i++) {
        for (int j = (i == 0 ? 15 : 31); j >= 0; j -= 4) {
            uint8_t nibble = (message[i] >> j) & 0xF;
            uint8_t symbol = ENCODE_4B5B[nibble];
            for (int k = 4; k >= 0; k--) {
                bool bit = (symbol >> k) & 1;
                last_level = (last_level == 0) ? 1 : 0;
                bmc_buffer[bmc_bit_count++] = last_level;
                if (bit) {
                    last_level = (last_level == 0) ? 1 : 0;
                }
                bmc_buffer[bmc_bit_count++] = last_level;
            }
        }
    }

    // CRC (dummy for now)
    for (int i = 0; i < 32; i++) {
        last_level = (last_level == 0) ? 1 : 0;
        bmc_buffer[bmc_bit_count++] = last_level;
        bmc_buffer[bmc_bit_count++] = last_level;
    }

    // EOP
    uint8_t eop = K_EOP;
    for (int k = 4; k >= 0; k--) {
        bool bit = (eop >> k) & 1;
        last_level = (last_level == 0) ? 1 : 0;
        bmc_buffer[bmc_bit_count++] = last_level;
        if (bit) {
            last_level = (last_level == 0) ? 1 : 0;
        }
        bmc_buffer[bmc_bit_count++] = last_level;
    }

    // Send to PIO
    for (int i = 0; i < bmc_bit_count; i++) {
        pio_sm_put_blocking(pio, sm, bmc_buffer[i]);
    }
}
