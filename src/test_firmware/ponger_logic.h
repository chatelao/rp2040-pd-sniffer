/**
 * @file ponger_logic.h
 * @brief Logic for the ponger.
 */
#ifndef PONGER_LOGIC_H
#define PONGER_LOGIC_H

#include "pd_library.h"

/**
 * @brief Processes a received packet and, if it's a "PING", prepares a "PONG" packet in response.
 * @param received_packet A pointer to the received packet.
 * @param response_packet A pointer to a packet to be filled with the response.
 * @return True if a response packet was created, false otherwise.
 */
bool ponger_process_packet(pd_packet_t* received_packet, pd_packet_t* response_packet);

#endif // PONGER_LOGIC_H
