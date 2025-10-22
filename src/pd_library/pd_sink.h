#ifndef PD_SINK_H
#define PD_SINK_H

#include "pd_library.h"

typedef enum {
    SINK_STATE_INIT,
    SINK_STATE_READY,
    SINK_STATE_REQUESTING,
} sink_state_t;

typedef struct {
    sink_state_t state;
    uint32_t message_id_counter;
    uint64_t state_timer;
    unsigned int tx_sm;
} pd_sink_t;

void sink_init(pd_sink_t* sink, unsigned int sm);
void sink_tick(pd_sink_t* sink);
void sink_handle_packet(pd_sink_t* sink, pd_packet_t* packet);

#endif // PD_SINK_H
