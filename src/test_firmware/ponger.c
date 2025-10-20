#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/clocks.h"
#include "pd_library.h"
#include "pd_receiver.pio.h"
#include "pd_transmitter.pio.h"

#define CC1_PIN 28
#define CC2_PIN 29
#define LED_PIN PICO_DEFAULT_LED_PIN

#define CAPTURE_BUF_SIZE 256

PIO pio_rx = pio0;
PIO pio_tx = pio1;
uint sm_rx, sm_tx;
uint dma_chan;
uint32_t capture_buf[CAPTURE_BUF_SIZE];

// Custom VDM for testing: "PING"
const uint32_t PING_VDM_HEADER = 0x0001; // Unstructured VDM
const uint32_t PING_VDM_DATA[] = {0x50494E47}; // "PING"

// Custom VDM for testing: "PONG"
const uint32_t PONG_VDM_HEADER = 0x0001; // Unstructured VDM
const uint32_t PONG_VDM_DATA[] = {0x504F4E47}; // "PONG"

void on_packet(pd_packet_t* packet) {
    if (packet->valid && (packet->header & 0x7FFF) == PING_VDM_HEADER) {
        if (packet->num_data_objects > 0 && packet->data[0] == PING_VDM_DATA[0]) {
            printf("PING received! Sending PONG...\n");
            gpio_put(LED_PIN, !gpio_get(LED_PIN)); // Toggle LED

            pd_packet_t pong_packet = {
                .header = PONG_VDM_HEADER,
                .num_data_objects = 1,
                .data = {PONG_VDM_DATA[0]}
            };
            pd_transmit_packet(pio_tx, sm_tx, &pong_packet);
        }
    }
}

void setup_pins() {
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // Receiver setup on CC1
    uint offset_rx = pio_add_program(pio_rx, &pd_receiver_program);
    sm_rx = pio_claim_unused_sm(pio_rx, true);
    pio_sm_config c_rx = pd_receiver_program_get_default_config(offset_rx);
    sm_config_set_in_pins(&c_rx, CC1_PIN);
    sm_config_set_jmp_pin(&c_rx, CC1_PIN);
    sm_config_set_in_shift(&c_rx, false, true, 32);
    sm_config_set_fifo_join(&c_rx, PIO_FIFO_JOIN_RX);
    pio_sm_init(pio_rx, sm_rx, offset_rx, &c_rx);
    pio_sm_set_enabled(pio_rx, sm_rx, true);

    // DMA setup for receiver
    dma_chan = dma_claim_unused_channel(true);
    dma_channel_config c_dma = dma_channel_get_default_config(dma_chan);
    channel_config_set_read_increment(&c_dma, false);
    channel_config_set_write_increment(&c_dma, true);
    channel_config_set_dreq(&c_dma, pio_get_dreq(pio_rx, sm_rx, false));
    dma_channel_configure(dma_chan, &c_dma, capture_buf, &pio_rx->rxf[sm_rx], CAPTURE_BUF_SIZE, true);

    // Transmitter setup on CC1
    pd_transmitter_init(pio_tx, sm_tx, CC1_PIN);
}

void ponger_loop() {
    static uint32_t read_index = 0;
    uint32_t dma_write_index = dma_hw->ch[dma_chan].write_addr / 4;
    dma_write_index = (dma_write_index - ((uint32_t)capture_buf / 4)) % CAPTURE_BUF_SIZE;

    while (read_index != dma_write_index) {
        pd_packet_t packet;
        pd_decode_packet(&capture_buf[read_index], 1, &packet);
        if (packet.valid) {
            on_packet(&packet);
        }
        read_index = (read_index + 1) % CAPTURE_BUF_SIZE;
    }
}

int main() {
    stdio_init_all();
    printf("Ponger running...\n");
    setup_pins();

    while (true) {
        ponger_loop();
    }
}
