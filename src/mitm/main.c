#include <stdio.h>
#include "pico/stdlib.h"
#include "pd_library.h"
#include "pd_sink.h"
#include "hal/hal.h"

static pd_sink_t sink0;
static pd_sink_t sink1;

/**
 * @brief Injects a Vendor Defined Message (VDM) into the USB-PD bus.
 * @param sm The state machine to use for transmission.
 */
static void inject_vdm(unsigned int sm) {
    pd_packet_t packet;
    packet.header = pd_header_build(1, 0xF, true, false, 2, 0);
    packet.num_data_objects = 1;
    packet.data[0] = pd_build_vdm_header(0x1234, true, 1, 0x5);
    pd_transmit_packet(sm, &packet);
}

int main() {
    stdio_init_all();

    unsigned int port0_sm = hal_init(0);
    unsigned int port1_sm = hal_init(1);

    sink_init(&sink0, port0_sm);
    sink_init(&sink1, port1_sm);

    sink_request_power(&sink0, 15000, 3000);

    while (true) {
        sink_tick(&sink0);
        sink_tick(&sink1);

        pd_packet_t packet;
        if (hal_get_packet(0, &packet)) {
            sink_handle_packet(&sink0, &packet);
        }
        if (hal_get_packet(1, &packet)) {
            sink_handle_packet(&sink1, &packet);
        }

        if (sink0.state == SINK_STATE_READY && sink1.state == SINK_STATE_READY) {
            inject_vdm(port1_sm);
        }
    }

    return 0;
}
