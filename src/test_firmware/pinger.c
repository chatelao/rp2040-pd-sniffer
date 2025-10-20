#include "pinger_logic.h"
#include "test_firmware_common.h"

#ifndef NATIVE_BUILD
#include <stdio.h>
#include "pico/stdlib.h"
#include "pd_library.h"
#include "pd_receiver.pio.h"
#include "pd_transmitter.pio.h"

#define CC1_PIN 28
#define CC2_PIN 29
#define LED_PIN PICO_DEFAULT_LED_PIN

PIO pio_rx = pio0;
PIO pio_tx = pio1;
uint sm_rx, sm_tx;

void on_packet(pd_packet_t* packet) {
    if (pinger_process_packet(packet)) {
        printf("PONG received!\n");
        gpio_put(LED_PIN, !gpio_get(LED_PIN)); // Toggle LED
    }
}

void setup_pins() {
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    pd_transmitter_init(pio_tx, sm_tx, CC1_PIN);
    // For simplicity, this test only transmits on CC1 and receives on CC2
}

int main() {
    stdio_init_all();
    printf("Pinger running...\n");

    setup_pins();

    pd_packet_t ping_packet = {
        .header = PING_VDM_HEADER,
        .num_data_objects = 1,
        .data = {PING_VDM_DATA[0]}
    };

    while (true) {
        printf("Sending PING...\n");
        pd_transmit_packet(pio_tx, sm_tx, &ping_packet);
        sleep_ms(1000);
    }
}
#endif // NATIVE_BUILD
