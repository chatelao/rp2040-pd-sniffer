#include "pico/stdlib.h"
#include "pd_library.h"
#include "pd_sink.h"
#include "hardware/dma.h"

#define RX_PIN 1

int main() {
    stdio_init_all();

    PIO pio = pio0;
    uint sm_rx = pio_claim_unused_sm(pio, true);
    pd_receiver_init(pio, sm_rx, RX_PIN);

    // Set up DMA to capture data from the PIO receiver
    uint dma_chan = dma_claim_unused_channel(true);
    dma_channel_config c = dma_channel_get_default_config(dma_chan);
    channel_config_set_read_increment(&c, false);
    channel_config_set_write_increment(&c, true);
    channel_config_set_dreq(&c, pio_get_dreq(pio, sm_rx, false));

    uint32_t captured_data[1024];
    dma_channel_configure(
        dma_chan,
        &c,
        captured_data,
        &pio->rxf[sm_rx],
        1024,
        false
    );

    sink_init();

    while (1) {
        sink_tick();

        // This is a placeholder for a more robust DMA-based receiver
        if (pio_sm_get_rx_fifo_level(pio, sm_rx) > 0) {
            uint32_t data_len = 1024 - dma_channel_hw_addr(dma_chan)->transfer_count;
            if (data_len > 0) {
                pd_packet_t packet;
                pd_decode_packet(captured_data, data_len, &packet);
                if (packet.valid) {
                    sink_handle_packet(pio, sm_rx, &packet);
                }
                dma_channel_set_write_addr(dma_chan, captured_data, true);
            }
        }
    }
}
