#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/clocks.h"
#include "pd_receiver.pio.h"
#include "pd_transmitter.pio.h"
#include "pd_library.h"

// --- Configuration ---
#define CC_PIN 28
#define SAMPLE_FREQ (300000 * 8)
#define CAPTURE_BUF_SIZE (1024 * 4)

// --- Globals ---
PIO pio_rx = pio0;
PIO pio_tx = pio1;
uint sm_rx;
uint sm_tx;
uint dma_chan;
__attribute__ ((aligned(4))) uint32_t capture_buf[CAPTURE_BUF_SIZE];

// --- Packet Callback ---
void on_packet_decoded(pd_packet_t *packet) {
    if (packet->valid && packet->num_data_objects > 0 && (packet->header & 0x1F) == 0x01) { // Source Capabilities
        for (int i = 0; i < packet->num_data_objects; i++) {
            uint32_t pdo = packet->data[i];
            if ((pdo >> 30) == 0) { // Fixed Supply
                uint16_t voltage = ((pdo >> 10) & 0x3FF) * 50;
                uint16_t current = (pdo & 0x3FF) * 10;
                if (voltage == 15000 && current >= 3000) {
                    printf("Found 15V/3A profile, requesting...\n");
                    pd_packet_t request_packet;
                    request_packet.header = pd_header_build(1, 2, 0, 0, 1, 2); // Request
                    request_packet.num_data_objects = 1;
                    request_packet.data[0] = (1 << 28) | (1 << 24) | (300 << 10) | (300); // 15V, 3A
                    pd_transmit_packet(pio_tx, sm_tx, &request_packet);
                    break;
                }
            }
        }
    }
}

// --- Initialization ---
void sink_init() {
    // RX
    pio_gpio_init(pio_rx, CC_PIN);
    uint offset_rx = pio_add_program(pio_rx, &pd_receiver_program);
    sm_rx = pio_claim_unused_sm(pio_rx, true);
    float div = (float)clock_get_hz(clk_sys) / (float)SAMPLE_FREQ;

    pio_sm_config c = pd_receiver_program_get_default_config(offset_rx);
    sm_config_set_in_pins(&c, CC_PIN);
    sm_config_set_in_shift(&c, false, true, 32);
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_RX);
    sm_config_set_clkdiv(&c, div);
    pio_sm_init(pio_rx, sm_rx, offset_rx, &c);

    dma_chan = dma_claim_unused_channel(true);
    dma_channel_config dma_c = dma_channel_get_default_config(dma_chan);
    channel_config_set_transfer_data_size(&dma_c, DMA_SIZE_32);
    channel_config_set_read_increment(&dma_c, false);
    channel_config_set_write_increment(&dma_c, true);
    channel_config_set_dreq(&dma_c, pio_get_dreq(pio_rx, sm_rx, false));
    channel_config_set_ring(&dma_c, true, 14);

    dma_channel_configure(dma_chan, &dma_c, capture_buf, &pio_rx->rxf[sm_rx], CAPTURE_BUF_SIZE, true);
    pio_sm_set_enabled(pio_rx, sm_rx, true);

    // TX
    pd_transmitter_init(pio_tx, sm_tx, CC_PIN);
}

// --- Main Loop ---
void sink_loop() {
    static uint32_t read_index = 0;
    uint32_t dma_write_index = dma_hw->ch[dma_chan].write_addr / 4;
    dma_write_index = (dma_write_index - ((uint32_t)capture_buf / 4)) % CAPTURE_BUF_SIZE;

    while (read_index != dma_write_index) {
        pd_packet_t packet;
        pd_decode_packet(&capture_buf[read_index], 1, &packet);
        if (packet.valid) {
            on_packet_decoded(&packet);
        }
        read_index = (read_index + 1) % CAPTURE_BUF_SIZE;
    }
    sleep_us(10);
}

int main() {
    stdio_init_all();
    printf("RP2040 PD Sink Initializing...\n");
    sink_init();
    printf("Sink running!\n");
    while (true) {
        sink_loop();
    }
}
