#include <stdio.h>
#include "pico/stdlib.h"
#include "pd_library.h"
#include "hal/hal.h"

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

int main() {
    stdio_init_all();
    printf("RP2040 PD Sniffer Initializing...\n");
    hal_init(0);
    hal_init(1);
    printf("Sniffer running!\n");
    while (true) {
        pd_packet_t packet;
        if (hal_get_packet(0, &packet)) {
            on_packet_decoded(&packet);
        }
        if (hal_get_packet(1, &packet)) {
            on_packet_decoded(&packet);
        }
    }
}
