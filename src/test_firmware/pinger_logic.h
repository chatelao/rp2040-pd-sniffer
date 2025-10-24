/**
 * @file pinger_logic.h
 * @brief Logic for the pinger.
 */
#ifndef PINGER_LOGIC_H
#define PINGER_LOGIC_H

#include "pd_library.h"
#include <stdbool.h>

/**
 * @brief Processes a received packet and returns true if it's a "PONG" message.
 * @param packet A pointer to the received packet.
 * @return True if the packet is a "PONG" message, false otherwise.
 */
bool pinger_process_packet(pd_packet_t* packet);

#endif // PINGER_LOGIC_H
