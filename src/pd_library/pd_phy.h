#ifndef PD_PHY_H
#define PD_PHY_H

#include "pd_library.h"

typedef struct {
    enum { STATE_SOP, STATE_PACKET } decoder_state;
    uint64_t shift_reg;
    int bit_count;
    pd_packet_t current_packet;
    uint8_t packet_buffer[34];
    int quintet_count;
    enum { PKT_STATE_HEADER, PKT_STATE_DATA, PKT_STATE_CRC } packet_state;
} pd_phy_decoder_t;

void pd_phy_decoder_init(pd_phy_decoder_t* decoder);
void pd_phy_decode_stream(pd_phy_decoder_t* decoder, const uint32_t* captured_data, uint32_t data_len, void (*callback)(void*, pd_packet_t*), uintptr_t context);

#endif // PD_PHY_H
