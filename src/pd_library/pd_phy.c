#include "pd_phy.h"
#include "pd_library.h"
#include <string.h>

/**
 * @brief 5b/4b decoding table.
 */
const uint8_t fiveb_to_fourb[32] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 4, 5, 8, 9, 6, 7,
    0, 0, 2, 3, 10, 11, 12, 13,
    0, 0, 0, 0, 14, 15, 0, 0,
};

#define K_CODE_SYNC1 0b11000 /**< K-code for Sync-1 */
#define K_CODE_SYNC2 0b10001 /**< K-code for Sync-2 */
#define K_CODE_EOP   0b01101 /**< K-code for End of Packet */

void pd_phy_decoder_init(pd_phy_decoder_t* decoder) {
    memset(decoder, 0, sizeof(pd_phy_decoder_t));
}

void pd_phy_decode_stream(pd_phy_decoder_t* decoder, const uint32_t* captured_data, uint32_t data_len, void (*callback)(void*, pd_packet_t*), uintptr_t context) {
    for (uint32_t i = 0; i < data_len; i++) {
        uint32_t word = captured_data[i];
        for (int j = 0; j < 32; j++) {
            int bit = (word >> (31 - j)) & 1;
            decoder->shift_reg = (decoder->shift_reg << 1) | bit;

            switch (decoder->decoder_state) {
                case STATE_SOP:
                    if ((decoder->shift_reg & 0xFFFFF) == ((K_CODE_SYNC1 << 15) | (K_CODE_SYNC1 << 10) | (K_CODE_SYNC2 << 5) | K_CODE_SYNC2)) {
                        decoder->decoder_state = STATE_PACKET;
                        decoder->packet_state = PKT_STATE_HEADER;
                        decoder->bit_count = 0;
                        decoder->quintet_count = 0;
                        memset(&decoder->current_packet, 0, sizeof(decoder->current_packet));
                        memset(decoder->packet_buffer, 0, sizeof(decoder->packet_buffer));
                    }
                    break;

                case STATE_PACKET:
                    decoder->bit_count++;
                    if (decoder->bit_count == 5) {
                        decoder->bit_count = 0;
                        uint8_t symbol = decoder->shift_reg & 0x1F;

                        if (symbol == K_CODE_EOP) {
                            int expected_quintets = 4 + decoder->current_packet.num_data_objects * 8 + 8;
                            if (decoder->quintet_count == expected_quintets) {
                                // Finalize packet
                                memcpy(&decoder->current_packet.header, decoder->packet_buffer, 2);
                                memcpy(decoder->current_packet.data, decoder->packet_buffer + 2, decoder->current_packet.num_data_objects * 4);
                                memcpy(&decoder->current_packet.crc, decoder->packet_buffer + 2 + decoder->current_packet.num_data_objects * 4, 4);

                                uint8_t crc_data[2 + MAX_DATA_OBJECTS * 4];
                                memcpy(crc_data, &decoder->current_packet.header, 2);
                                if (decoder->current_packet.num_data_objects > 0) {
                                   memcpy(crc_data + 2, decoder->current_packet.data, decoder->current_packet.num_data_objects * 4);
                                }
                                uint32_t calculated_crc = pd_crc32(crc_data, 2 + decoder->current_packet.num_data_objects * 4);

                                if (calculated_crc == decoder->current_packet.crc) {
                                    decoder->current_packet.valid = true;
                                    callback((void*)context, &decoder->current_packet);
                                }
                            }
                            decoder->decoder_state = STATE_SOP;
                        } else {
                            uint8_t nybble = fiveb_to_fourb[symbol];
                            int byte_pos = decoder->quintet_count / 2;
                            if (decoder->quintet_count < sizeof(decoder->packet_buffer) * 2) {
                                if (decoder->quintet_count % 2 == 0) decoder->packet_buffer[byte_pos] = nybble;
                                else decoder->packet_buffer[byte_pos] |= (nybble << 4);
                            }
                            decoder->quintet_count++;

                            if (decoder->packet_state == PKT_STATE_HEADER && decoder->quintet_count == 4) {
                                memcpy(&decoder->current_packet.header, decoder->packet_buffer, 2);
                                decoder->current_packet.num_data_objects = (decoder->current_packet.header >> 12) & 0x7;
                                if (decoder->current_packet.num_data_objects > 7) decoder->current_packet.num_data_objects = 0;
                                decoder->packet_state = (decoder->current_packet.num_data_objects == 0) ? PKT_STATE_CRC : PKT_STATE_DATA;
                            } else if (decoder->packet_state == PKT_STATE_DATA && decoder->quintet_count == 4 + decoder->current_packet.num_data_objects * 8) {
                                decoder->packet_state = PKT_STATE_CRC;
                            }
                        }
                    }
                    break;
            }
        }
    }
}
