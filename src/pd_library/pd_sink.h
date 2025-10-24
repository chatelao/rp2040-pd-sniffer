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

    // Desired capabilities
    uint32_t desired_voltage_mv;
    uint32_t desired_current_ma;

    // Source capabilities
    pd_packet_t source_capabilities;

    // Active power profile
    int active_voltage_mv;
    int active_current_ma;

    // Requested power profile
    int requested_object_position;
} pd_sink_t;

void sink_init(pd_sink_t* sink, unsigned int sm);
void sink_tick(pd_sink_t* sink);
void sink_handle_packet(pd_sink_t* sink, const pd_packet_t* packet);
void sink_request_power(pd_sink_t* sink, uint32_t voltage_mv, uint32_t current_ma);

#endif // PD_SINK_H
