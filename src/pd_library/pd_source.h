#ifndef PD_SOURCE_H
#define PD_SOURCE_H

#include "pd_library.h"

typedef enum {
    SOURCE_STATE_INIT,
    SOURCE_STATE_READY,
    SOURCE_STATE_NEGOTIATING,
    SOURCE_STATE_PROPRIETARY,
} source_state_t;

#ifndef NATIVE_BUILD
#include "hardware/pio.h"
#endif

void source_init(void);
void source_tick(PIO pio, uint sm);
void source_handle_packet(PIO pio, uint sm, pd_packet_t* packet);
source_state_t source_get_state(void);
void source_send_vdm(PIO pio, uint sm);

#endif // PD_SOURCE_H
