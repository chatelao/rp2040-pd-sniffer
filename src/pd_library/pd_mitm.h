#ifndef PD_MITM_H
#define PD_MITM_H

#include "pd_library.h"

typedef enum {
    MITM_STATE_INIT,
    MITM_STATE_NEGOTIATING_SOURCE,
    MITM_STATE_READY_TO_SINK,
    MITM_STATE_NEGOTIATING_SINK,
} mitm_state_t;

void mitm_init(void);
void mitm_tick(PIO pio_source, uint sm_source, PIO pio_sink, uint sm_sink);
void mitm_handle_packet_from_source(PIO pio_source, uint sm_source, pd_packet_t* packet);
void mitm_handle_packet_from_sink(PIO pio_sink, uint sm_sink, pd_packet_t* packet);

#endif // PD_MITM_H
