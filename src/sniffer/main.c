#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/clocks.h"
#include "pd_receiver.pio.h"
#include "pd_library.h"

// --- Configuration ---
#define CC_PIN 28
#define SAMPLE_FREQ (300000 * 8)
#define CAPTURE_BUF_SIZE (1024 * 4)

// --- Globals ---
PIO pio = pio0;
uint sm;
uint dma_chan;
__attribute__ ((aligned(4))) uint32_t capture_buf[CAPTURE_BUF_SIZE];

// --- Packet Callback ---
void on_packet_decoded(pd_packet_t *packet) {
    printf("--- PD Packet ---\n");
    printf("Header: 0x%04x\n", packet->header);
    printf("Data Objs: %d\n", packet->num_data_objects);
    for (int i = 0; i < packet->num_data_objects; i++) {
        printf("  Data[%d]: 0x%08x\n", i, packet->data[i]);
    }
    printf("CRC: 0x%08x\n", packet->crc);
    printf("-----------------\n\n");
}

// --- Initialization ---
void sniffer_init() {
    pio_gpio_init(pio, CC_PIN);
    uint offset = pio_add_program(pio, &pd_receiver_program);
    sm = pio_claim_unused_sm(pio, true);
    float div = (float)clock_get_hz(clk_sys) / (float)SAMPLE_FREQ;

    pio_sm_config c = pd_receiver_program_get_default_config(offset);
    sm_config_set_in_pins(&c, CC_PIN);
    sm_config_set_in_shift(&c, false, true, 32);
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_RX);
    sm_config_set_clkdiv(&c, div);
    pio_sm_init(pio, sm, offset, &c);

    dma_chan = dma_claim_unused_channel(true);
    dma_channel_config dma_c = dma_channel_get_default_config(dma_chan);
    channel_config_set_transfer_data_size(&dma_c, DMA_SIZE_32);
    channel_config_set_read_increment(&dma_c, false);
    channel_config_set_write_increment(&dma_c, true);
    channel_config_set_dreq(&dma_c, pio_get_dreq(pio, sm, false));
    channel_config_set_ring(&dma_c, true, 14);

    dma_channel_configure(dma_chan, &dma_c, capture_buf, &pio->rxf[sm], CAPTURE_BUF_SIZE, true);
    pio_sm_set_enabled(pio, sm, true);
}

// --- Main Loop ---
void sniffer_loop() {
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
    printf("RP2040 PD Sniffer Initializing...\n");
    sniffer_init();
    printf("Sniffer running!\n");
    while (true) {
        sniffer_loop();
    }
}
