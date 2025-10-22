#include "pd_mitm.h"
#include "pd_sink.h"
#include "pd_source.h"
#include <string.h>

#ifdef NATIVE_BUILD
mitm_state_t state;
#else
static mitm_state_t state;
#endif

void mitm_init(void) {
    state = MITM_STATE_INIT;
    sink_init();
    source_init();
}

void mitm_tick(PIO pio_source, uint sm_source, PIO pio_sink, uint sm_sink) {
    sink_tick();
    source_tick(pio_source, sm_source);

    if (state == MITM_STATE_INIT) {
        if (sink_get_state() == SINK_STATE_READY) {
            // TODO: In a real implementation, we would parse the source capabilities
            // and select an appropriate one. For now, we just request the first one.
            send_request(pio_sink, sm_sink, 1);
            state = MITM_STATE_NEGOTIATING_SOURCE;
        }
    } else if (state == MITM_STATE_NEGOTIATING_SOURCE) {
        if (sink_get_state() == SINK_STATE_CONTRACT_ESTABLISHED) {
            state = MITM_STATE_READY_TO_SINK;
        }
    } else if (state == MITM_STATE_READY_TO_SINK) {
        if (source_get_state() == SOURCE_STATE_READY) {
            source_send_vdm(pio_source, sm_source);
            state = MITM_STATE_NEGOTIATING_SINK;
        }
    }
}

void mitm_handle_packet_from_source(PIO pio_source, uint sm_source, pd_packet_t* packet) {
    sink_handle_packet(pio_source, sm_source, packet);
}

void mitm_handle_packet_from_sink(PIO pio_sink, uint sm_sink, pd_packet_t* packet) {
    source_handle_packet(pio_sink, sm_sink, packet);
}
