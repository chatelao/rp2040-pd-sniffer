#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/clocks.h"
#include "pd_transmitter.pio.h"
#include "pd_sniffer.pio.h"
#include "../pd_library/pd_library.h"

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
    if (packet->valid && packet->num_data_objs > 0 && (packet->header & 0x1F) == 0x01) { // Source Capabilities
        for (int i = 0; i < packet->num_data_objs; i++) {
            uint32_t pdo = packet->data[i];
            if ((pdo >> 30) == 0) { // Fixed Supply
                uint16_t voltage = ((pdo >> 10) & 0x3FF) * 50;
                uint16_t current = (pdo & 0x3FF) * 10;
                if (voltage == 15000 && current >= 3000) {
                    printf("Found 15V/3A profile, requesting...\n");
                    uint16_t header = pd_build_request_header(1, 0, 0, 1, 0, 0x02); // Request
                    uint32_t rdo = (1 << 28) | (1 << 24) | (300 << 10) | (300); // 15V, 3A
                    pd_transmit_packet(pio_tx, sm_tx, header, &rdo);
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
    uint offset_rx = pio_add_program(pio_rx, &pd_sampler_program);
    sm_rx = pio_claim_unused_sm(pio_rx, true);
    float div = (float)clock_get_hz(clk_sys) / (float)SAMPLE_FREQ;
    pd_sampler_program_init(pio_rx, sm_rx, offset_rx, CC_PIN, div);

    dma_chan = dma_claim_unused_channel(true);
    dma_channel_config c = dma_channel_get_default_config(dma_chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
    channel_config_set_read_increment(&c, false);
    channel_config_set_write_increment(&c, true);
    channel_config_set_dreq(&c, pio_get_dreq(pio_rx, sm_rx, false));
    channel_config_set_ring(&c, true, 14);

    dma_channel_configure(dma_chan, &c, capture_buf, &pio_rx->rxf[sm_rx], CAPTURE_BUF_SIZE, true);
    pio_sm_set_enabled(pio_rx, sm_rx, true);

    // TX
    uint offset_tx = pio_add_program(pio_tx, &pd_transmitter_program);
    sm_tx = pio_claim_unused_sm(pio_tx, true);
    pd_transmitter_program_init(pio_tx, sm_tx, offset_tx, CC_PIN);
    pio_sm_set_enabled(pio_tx, sm_tx, true);
}

// --- Main Loop ---
void sink_loop() {
    static uint32_t read_index = 0;
    uint32_t dma_write_index = dma_hw->ch[dma_chan].write_addr / 4;
    dma_write_index = (dma_write_index - ((uint32_t)capture_buf / 4)) % CAPTURE_BUF_SIZE;

    while (read_index != dma_write_index) {
        bmc_decoder_feed(capture_buf[read_index], on_packet_decoded);
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
