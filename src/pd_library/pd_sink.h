#ifndef PD_SINK_H
#define PD_SINK_H

#include "pd_library.h"

typedef enum {
    SINK_STATE_INIT,
    SINK_STATE_READY,
    SINK_STATE_REQUESTING,
    SINK_STATE_CONTRACT_ESTABLISHED,
} sink_state_t;

void sink_init(void);
void sink_tick(void);
void sink_handle_packet(PIO pio, uint sm, pd_packet_t* packet);
sink_state_t sink_get_state(void);
void send_request(PIO pio, uint sm, int object_position);

#endif // PD_SINK_H
