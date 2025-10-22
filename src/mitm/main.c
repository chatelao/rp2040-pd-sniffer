#include <stdio.h>
#include "pico/stdlib.h"
#include "pd_library.h"
#include "pd_sink.h"
#include "pd_source.h"
#include "hal/hal.h"

// Shared state between sink and source
volatile bool power_ready = false;

// Custom sink handler to notify the source when power is ready
void mitm_sink_handle_packet(pd_packet_t* packet) {
    sink_handle_packet(packet);
    if (sink_get_state() == SINK_STATE_READY && !power_ready) {
        power_ready = true;
    }
}

int main() {
    stdio_init_all();

    unsigned int sink_sm = hal_init(0);
    unsigned int source_sm = hal_init(1);

    sink_init(sink_sm);
    source_init(source_sm);

    while (true) {
        sink_tick();
        if (power_ready) {
            source_tick();
        }

        pd_packet_t packet;
        if (hal_get_packet(0, &packet)) {
            mitm_sink_handle_packet(&packet);
        }
        if (hal_get_packet(1, &packet) && power_ready) {
            source_handle_packet(&packet);
        }
    }

    return 0;
}
