#ifndef PD_SINK_H
#define PD_SINK_H

#include "pd_library.h"

typedef enum {
    SINK_STATE_INIT,
    SINK_STATE_READY,
    SINK_STATE_REQUESTING,
} sink_state_t;

void sink_init(unsigned int sm);
void sink_tick(void);
void sink_handle_packet(pd_packet_t* packet);

#endif // PD_SINK_H
