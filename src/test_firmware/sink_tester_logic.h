#ifndef SINK_TESTER_LOGIC_H
#define SINK_TESTER_LOGIC_H

#include "pd_library.h"
#include <stdbool.h>

// This function processes a received Source_Capabilities packet and, if it matches
// the desired profile, prepares a Request packet in response.
// Returns true if a response packet was created.
bool sink_tester_process_packet(pd_packet_t* received_packet, pd_packet_t* response_packet);

#endif // SINK_TESTER_LOGIC_H
