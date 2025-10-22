#include "hal/hal.h"
#include "pd_library.h"
#include "pd_sink.h"

#define RX_PIN 1
#define TX_PIN 2

int main() {
    hal_gpio_init(RX_PIN);
    hal_gpio_set_dir(RX_PIN, false);
    hal_gpio_init(TX_PIN);
    hal_gpio_set_dir(TX_PIN, true);

    hal_dma_init();
    hal_dma_start_transfer(NULL, NULL, 0);

    hal_pio_init();
    hal_pio_sm_init(0, RX_PIN);
    hal_pio_sm_init(4, TX_PIN);

    sink_init(4); // Pass the transmitter state machine

    while (1) {
        sink_tick();

        uint32_t captured_data[1024];
        uint32_t data_len = hal_dma_read(captured_data, 1024);

        if (data_len > 0) {
            pd_packet_t packet;
            pd_decode_packet(captured_data, data_len, &packet);
            if (packet.valid) {
                sink_handle_packet(&packet);
            }
        }
    }
}
