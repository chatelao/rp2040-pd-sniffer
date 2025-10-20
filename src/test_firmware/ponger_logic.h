#ifndef PONGER_LOGIC_H
#define PONGER_LOGIC_H

#include "pd_library.h"

// This function processes a received packet and, if it's a "PING",
// prepares a "PONG" packet in response.
// Returns true if a response packet was created.
bool ponger_process_packet(pd_packet_t* received_packet, pd_packet_t* response_packet);

#endif // PONGER_LOGIC_H
