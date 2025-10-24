/**
 * @file pd_library.h
 * @brief Shared library for USB Power Delivery.
 */
#ifndef PD_LIBRARY_H
#define PD_LIBRARY_H

#include "pd_common.h"

#ifdef NATIVE_BUILD
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#else
#include "pico/stdlib.h"
#include "hardware/pio.h"
#endif

#define MAX_DATA_OBJECTS 7

/**
 * @brief Represents a USB-PD packet.
 */
typedef struct {
    uint16_t header;                /**< The packet header. */
    uint32_t data[MAX_DATA_OBJECTS];/**< The packet data objects. */
    uint32_t crc;                   /**< The packet CRC. */
    int num_data_objects;           /**< The number of data objects. */
    bool valid;                     /**< True if the packet is valid. */
} pd_packet_t;

#ifdef NATIVE_BUILD
/**
 * @brief A callback function for handling decoded packets.
 */
typedef void (*packet_callback_t)(pd_packet_t* packet);

/**
 * @brief Resets the BMC decoder.
 */
void bmc_decoder_reset(void);

/**
 * @brief Feeds raw data to the BMC decoder.
 * @param raw_data The raw data to feed.
 * @param callback The callback function to call when a packet is decoded.
 */
void bmc_decoder_feed(uint32_t raw_data, packet_callback_t callback);
#endif

/**
 * @brief Calculates the CRC32 of a block of data.
 * @param data The data to calculate the CRC32 of.
 * @param len The length of the data in bytes.
 * @return The CRC32 of the data.
 */
uint32_t pd_crc32(const uint8_t *data, size_t len);

/**
 * @brief Encodes a USB-PD packet.
 * @param packet The packet to encode.
 * @param encoded_data A pointer to a buffer to store the encoded data.
 * @param encoded_len A pointer to a variable to store the length of the encoded data.
 */
void pd_encode_packet(const pd_packet_t* packet, uint32_t* encoded_data, size_t* encoded_len);

/**
 * @brief Transmits a USB-PD packet.
 * @param sm The state machine to use for transmission.
 * @param packet The packet to transmit.
 */
void pd_transmit_packet(unsigned int sm, const pd_packet_t* packet);

/**
 * @brief Builds a USB-PD header.
 * @param num_data_objects The number of data objects in the packet.
 * @param message_type The message type.
 * @param port_power_role The port power role.
 * @param port_data_role The port data role.
 * @param spec_rev The USB-PD specification revision.
 * @param message_id The message ID.
 * @return The USB-PD header.
 */
uint16_t pd_header_build(int num_data_objects, uint16_t message_type, bool port_power_role, bool port_data_role, uint8_t spec_rev, uint8_t message_id);

/**
 * @brief Builds a Vendor Defined Message (VDM) header.
 * @param vendor_id The vendor ID.
 * @param structured True if the VDM is structured, false otherwise.
 * @param command_type The command type.
 * @param command The command.
 * @return The VDM header.
 */
uint32_t pd_build_vdm_header(uint16_t vendor_id, bool structured, uint8_t command_type, uint8_t command);

#endif // PD_LIBRARY_H
