#include "pico/stdlib.h"
#include "pd_library.h"
#include "pd_mitm.h"
#include "hardware/dma.h"

#define SOURCE_RX_PIN 1
#define SOURCE_TX_PIN 0
#define SINK_RX_PIN 3
#define SINK_TX_PIN 2

int main() {
    stdio_init_all();

    PIO pio_source = pio0;
    PIO pio_sink = pio1;
    uint sm_source_rx = pio_claim_unused_sm(pio_source, true);
    uint sm_source_tx = pio_claim_unused_sm(pio_source, true);
    uint sm_sink_rx = pio_claim_unused_sm(pio_sink, true);
    uint sm_sink_tx = pio_claim_unused_sm(pio_sink, true);

    pd_receiver_init(pio_source, sm_source_rx, SOURCE_RX_PIN);
    pd_transmitter_init(pio_source, sm_source_tx, SOURCE_TX_PIN);
    pd_receiver_init(pio_sink, sm_sink_rx, SINK_RX_PIN);
    pd_transmitter_init(pio_sink, sm_sink_tx, SINK_TX_PIN);

    // Set up DMA for both receivers
    uint dma_chan_source = dma_claim_unused_channel(true);
    uint dma_chan_sink = dma_claim_unused_channel(true);

    // DMA config for source
    dma_channel_config c_source = dma_channel_get_default_config(dma_chan_source);
    channel_config_set_read_increment(&c_source, false);
    channel_config_set_write_increment(&c_source, true);
    channel_config_set_dreq(&c_source, pio_get_dreq(pio_source, sm_source_rx, false));
    uint32_t captured_data_source[1024];
    dma_channel_configure(dma_chan_source, &c_source, captured_data_source, &pio_source->rxf[sm_source_rx], 1024, false);

    // DMA config for sink
    dma_channel_config c_sink = dma_channel_get_default_config(dma_chan_sink);
    channel_config_set_read_increment(&c_sink, false);
    channel_config_set_write_increment(&c_sink, true);
    channel_config_set_dreq(&c_sink, pio_get_dreq(pio_sink, sm_sink_rx, false));
    uint32_t captured_data_sink[1024];
    dma_channel_configure(dma_chan_sink, &c_sink, captured_data_sink, &pio_sink->rxf[sm_sink_rx], 1024, false);

    mitm_init();

    // Placeholder for interrupt handler
    void pio_irq_handler() {
        // This is a placeholder for a more robust interrupt-based receiver
        // In a real implementation, we would check which PIO fired the IRQ
        // and process the corresponding FIFO.
    }

    irq_set_exclusive_handler(PIO0_IRQ_0, pio_irq_handler);
    irq_set_enabled(PIO0_IRQ_0, true);
    irq_set_exclusive_handler(PIO1_IRQ_0, pio_irq_handler);
    irq_set_enabled(PIO1_IRQ_0, true);

    while (1) {
        mitm_tick(pio_source, sm_source_tx, pio_sink, sm_sink_tx);
    }
}
