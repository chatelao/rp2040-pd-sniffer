#include "source_emulator_logic.h"

#ifndef NATIVE_BUILD
#include <stdio.h>
#include "pico/stdlib.h"
#include "pd_library/pd_library.h"
#include "pd_transmitter.pio.h"

#define CC1_PIN 28
#define CC2_PIN 29
#define LED_PIN PICO_DEFAULT_LED_PIN

PIO pio_tx = pio0;
uint sm_tx;

void setup_pins() {
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    pd_transmitter_init(pio_tx, sm_tx, CC1_PIN);
}

int main() {
    stdio_init_all();
    printf("Source Emulator running...\n");

    setup_pins();

    pd_packet_t source_cap_packet;
    source_emulator_prepare_packet(&source_cap_packet);

    while (true) {
        printf("Broadcasting Source_Capabilities...\n");
        gpio_put(LED_PIN, !gpio_get(LED_PIN)); // Toggle LED
        pd_transmit_packet(pio_tx, sm_tx, &source_cap_packet);
        sleep_ms(2000);
    }
}
#endif // NATIVE_BUILD
