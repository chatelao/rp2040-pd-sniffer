#ifndef PD_SOURCE_H
#define PD_SOURCE_H

#include "pd_library.h"

typedef enum {
    SOURCE_STATE_INIT,
    SOURCE_STATE_READY,
    SOURCE_STATE_REQUEST_RECEIVED,
} source_state_t;

void source_init(unsigned int sm);
void source_tick(void);
void source_handle_packet(pd_packet_t* packet);

#endif // PD_SOURCE_H
