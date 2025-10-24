/**
 * @file pd_phy.h
 * @brief USB Power Delivery physical layer.
 */
#ifndef PD_PHY_H
#define PD_PHY_H

#include "pd_library.h"

/**
 * @brief Represents a USB-PD physical layer decoder.
 */
typedef struct {
    enum { STATE_SOP, STATE_PACKET } decoder_state;  /**< The state of the decoder. */
    uint64_t shift_reg;                             /**< The decoder's shift register. */
    int bit_count;                                  /**< The number of bits in the shift register. */
    pd_packet_t current_packet;                     /**< The packet currently being decoded. */
    uint8_t packet_buffer[34];                      /**< A buffer for the packet data. */
    int quintet_count;                              /**< The number of quintets decoded. */
    enum { PKT_STATE_HEADER, PKT_STATE_DATA, PKT_STATE_CRC } packet_state;/**< The state of the packet decoding process. */
} pd_phy_decoder_t;

/**
 * @brief Initializes a USB-PD physical layer decoder.
 * @param decoder A pointer to the decoder to initialize.
 */
void pd_phy_decoder_init(pd_phy_decoder_t* decoder);

/**
 * @brief Decodes a stream of USB-PD data.
 * @param decoder A pointer to the decoder to use.
 * @param captured_data A pointer to the captured data.
 * @param data_len The length of the captured data.
 * @param callback A callback function to call when a packet is decoded.
 * @param context A context pointer to pass to the callback function.
 */
void pd_phy_decode_stream(pd_phy_decoder_t* decoder, const uint32_t* captured_data, uint32_t data_len, void (*callback)(void*, pd_packet_t*), uintptr_t context);

#endif // PD_PHY_H
