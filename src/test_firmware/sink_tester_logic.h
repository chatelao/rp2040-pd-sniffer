/**
 * @file sink_tester_logic.h
 * @brief Logic for the sink tester.
 */
#ifndef SINK_TESTER_LOGIC_H
#define SINK_TESTER_LOGIC_H

#include "pd_library.h"
#include <stdbool.h>

/**
 * @brief Processes a received Source_Capabilities packet and, if it matches the desired profile, prepares a Request packet in response.
 * @param received_packet A pointer to the received packet.
 * @param response_packet A pointer to a packet to be filled with the response.
 * @return True if a response packet was created, false otherwise.
 */
bool sink_tester_process_packet(pd_packet_t* received_packet, pd_packet_t* response_packet);

#endif // SINK_TESTER_LOGIC_H
