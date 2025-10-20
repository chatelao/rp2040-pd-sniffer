#ifndef PINGER_LOGIC_H
#define PINGER_LOGIC_H

#include "pd_library.h"
#include <stdbool.h>

// This function processes a received packet and returns true if it's a "PONG" message.
bool pinger_process_packet(pd_packet_t* packet);

#endif // PINGER_LOGIC_H
