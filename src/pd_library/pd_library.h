#ifndef PD_LIBRARY_H
#define PD_LIBRARY_H

#ifdef NATIVE_BUILD
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#else
#include "pico/stdlib.h"
#include "hardware/pio.h"
#endif

#define MAX_DATA_OBJECTS 7

typedef struct {
    uint16_t header;
    uint32_t data[MAX_DATA_OBJECTS];
    uint32_t crc;
    int num_data_objects;
    bool valid;
} pd_packet_t;

#ifdef NATIVE_BUILD
typedef void (*packet_callback_t)(pd_packet_t* packet);
void bmc_decoder_reset(void);
void bmc_decoder_feed(uint32_t raw_data, packet_callback_t callback);
#endif

uint32_t pd_crc32(const uint8_t *data, size_t len);
void pd_decode_packet(uint32_t* captured_data, uint32_t data_len, pd_packet_t* packet);
void pd_encode_packet(pd_packet_t* packet, uint32_t* encoded_data, size_t* encoded_len);

#ifndef NATIVE_BUILD
void pd_transmitter_init(PIO pio, uint sm, uint pin);
void pd_receiver_init(PIO pio, uint sm, uint pin);
void pd_transmit_packet(PIO pio, uint sm, pd_packet_t* packet);
#endif

uint16_t pd_header_build(int num_data_objects, uint16_t message_type, bool port_power_role, bool port_data_role, uint8_t spec_rev, uint8_t message_id);

#endif // PD_LIBRARY_H
