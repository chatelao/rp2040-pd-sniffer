#ifndef PD_LIBRARY_H
#define PD_LIBRARY_H

#include "pico/stdlib.h"
#include "hardware/pio.h"

// --- Public Data Structures ---

// Decoder state machine
typedef enum {
    STATE_IDLE,
    STATE_PREAMBLE,
    STATE_SOP,
    STATE_HEADER,
    STATE_DATA,
    STATE_CRC,
    STATE_EOP
} decoder_state_t;

// Structure to hold our decoded packet
typedef struct {
    uint16_t header;
    uint32_t data[7]; // Max 7 data objects
    uint32_t crc;
    uint8_t num_data_objs;
    bool valid;
} pd_packet_t;

// --- Public Function Prototypes ---

// Processes a block of 32 raw samples from the DMA buffer
void bmc_decoder_feed(uint32_t sample_word, void (*packet_callback)(pd_packet_t*));

// --- Transmission Functions ---

// Construct a USB-PD request message
uint16_t pd_build_request_header(uint8_t num_data_objs, uint8_t message_id, uint8_t power_role, uint8_t spec_rev, uint8_t data_role, uint8_t message_type);

// Transmit a USB-PD packet
void pd_transmit_packet(PIO pio, uint sm, uint16_t header, const uint32_t *data);

#endif // PD_LIBRARY_H
