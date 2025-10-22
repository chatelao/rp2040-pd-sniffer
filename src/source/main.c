#include "pico/stdlib.h"
#include "pd_library.h"
#include "pd_source.h"
#include "hardware/dma.h"

#define RX_PIN 1
#define TX_PIN 0

int main() {
    stdio_init_all();

    PIO pio = pio0;
    uint sm_rx = pio_claim_unused_sm(pio, true);
    uint sm_tx = pio_claim_unused_sm(pio, true);
    pd_receiver_init(pio, sm_rx, RX_PIN);
    pd_transmitter_init(pio, sm_tx, TX_PIN);

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

    source_init();

    // Placeholder for interrupt handler
    void pio_irq_handler() {
        // This is a placeholder for a more robust interrupt-based receiver
    }

    irq_set_exclusive_handler(PIO0_IRQ_0, pio_irq_handler);
    irq_set_enabled(PIO0_IRQ_0, true);

    while (1) {
        source_tick(pio, sm_tx);
    }
}
