#include "hal/hal.h"
#include "pd_library.h"
#include "pd_sink.h"

#define PORT 0

int main() {
    pd_sink_t sink;

    unsigned int sm_tx = hal_init(PORT);
    sink_init(&sink, sm_tx);

    while (1) {
        sink_tick(&sink);

        pd_packet_t packet;
        if (hal_get_packet(PORT, &packet)) {
            sink_handle_packet(&sink, &packet);
        }
    }
}
