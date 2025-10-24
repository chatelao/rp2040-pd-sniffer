#include "hal/hal.h"
#include "pd_phy.h"
#include <string.h>
#ifndef NATIVE_BUILD
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "hardware/pio.h"
#include "pd_receiver.pio.h"
#include "pd_transmitter.pio.h"
#endif

struct pd_packet_s;

// GPIO
void hal_gpio_init(unsigned int pin) {
#ifndef NATIVE_BUILD
    gpio_init(pin);
#endif
}

void hal_gpio_set_dir(unsigned int pin, bool out) {
#ifndef NATIVE_BUILD
    gpio_set_dir(pin, out);
#endif
}

void hal_gpio_put(unsigned int pin, bool value) {
#ifndef NATIVE_BUILD
    gpio_put(pin, value);
#endif
}

// DMA
#define DMA_BUFFER_SIZE 1024
uint32_t dma_buffer[DMA_BUFFER_SIZE];
#ifndef NATIVE_BUILD
static unsigned int dma_channel;
static PIO pio;
static unsigned int sm;
#endif

static volatile uint32_t write_pos = 0;
static volatile uint32_t read_pos = 0;

#ifndef NATIVE_BUILD
static void dma_irq_handler() {
    if (dma_channel_get_irq0_status(dma_channel)) {
        dma_channel_acknowledge_irq0(dma_channel);
        write_pos = (write_pos + 1) % DMA_BUFFER_SIZE;
        dma_channel_set_write_addr(dma_channel, &dma_buffer[write_pos], true);
    }
}
#else
void hal_dma_irq_handler_mock(void) {
    write_pos = (write_pos + 1) % DMA_BUFFER_SIZE;
}
#endif

void hal_dma_init(void) {
#ifndef NATIVE_BUILD
    dma_channel = dma_claim_unused_channel(true);
    pio = pio0;
    sm = pio_claim_unused_sm(pio, true);

    uint offset = pio_add_program(pio, &pd_receiver_program);
    pio_sm_config c = pd_receiver_program_get_default_config(offset);
    sm_config_set_in_pins(&c, 1); // Hardcoded pin for now
    sm_config_set_jmp_pin(&c, 1); // Hardcoded pin for now
    sm_config_set_in_shift(&c, false, true, 32);
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_RX);
    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_enabled(pio, sm, true);

    dma_channel_config dma_config = dma_channel_get_default_config(dma_channel);
    channel_config_set_read_increment(&dma_config, false);
    channel_config_set_write_increment(&dma_config, true);
    channel_config_set_dreq(&dma_config, pio_get_dreq(pio, sm, false));
    dma_channel_configure(
        dma_channel,
        &dma_config,
        dma_buffer,
        &pio->rxf[sm],
        DMA_BUFFER_SIZE,
        false
    );

    dma_channel_set_irq0_enabled(dma_channel, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_irq_handler);
    irq_set_enabled(DMA_IRQ_0, true);
#else
    write_pos = 0;
    read_pos = 0;
#endif
}

void hal_dma_start_transfer(uint32_t* src, uint32_t* dest, size_t len) {
#ifndef NATIVE_BUILD
    dma_channel_start(dma_channel);
#endif
}

uint32_t hal_dma_read(uint32_t* buffer, uint32_t len) {
    uint32_t count = 0;
    while (read_pos != write_pos && count < len) {
        buffer[count++] = dma_buffer[read_pos];
        read_pos = (read_pos + 1) % DMA_BUFFER_SIZE;
    }
    return count;
}


// CRC
uint32_t hal_crc32(const uint8_t *data, size_t len) {
    return pd_crc32(data, len);
}

// PIO / State Machine
#ifndef NATIVE_BUILD
static PIO pio_instances[2] = {pio0, pio1};
#endif

#define NUM_PORTS 2
#define DMA_BUFFER_SIZE_PER_PORT 1024

typedef struct {
    uint32_t dma_buffer[DMA_BUFFER_SIZE_PER_PORT];
#ifndef NATIVE_BUILD
    PIO pio;
#endif
    unsigned int sm_rx;
    unsigned int sm_tx;
    unsigned int dma_channel;
    volatile uint32_t write_pos;
    volatile uint32_t read_pos;
    pd_phy_decoder_t decoder;
} port_t;

static port_t ports[NUM_PORTS];

#ifndef NATIVE_BUILD
static void dma_irq_handler_port0() {
    if (dma_channel_get_irq0_status(ports[0].dma_channel)) {
        dma_channel_acknowledge_irq0(ports[0].dma_channel);
        ports[0].write_pos = (ports[0].write_pos + 1) % DMA_BUFFER_SIZE_PER_PORT;
        dma_channel_set_write_addr(ports[0].dma_channel, &ports[0].dma_buffer[ports[0].write_pos], true);
    }
}

static void dma_irq_handler_port1() {
    if (dma_channel_get_irq0_status(ports[1].dma_channel)) {
        dma_channel_acknowledge_irq0(ports[1].dma_channel);
        ports[1].write_pos = (ports[1].write_pos + 1) % DMA_BUFFER_SIZE_PER_PORT;
        dma_channel_set_write_addr(ports[1].dma_channel, &ports[1].dma_buffer[ports[1].write_pos], true);
    }
}

static void (*dma_irq_handlers[NUM_PORTS])() = {
    dma_irq_handler_port0,
    dma_irq_handler_port1
};
#endif

unsigned int hal_init(unsigned int port) {
#ifndef NATIVE_BUILD
    ports[port].pio = (port == 0) ? pio0 : pio1;
    ports[port].sm_rx = pio_claim_unused_sm(ports[port].pio, true);
    ports[port].sm_tx = pio_claim_unused_sm(ports[port].pio, true);
    ports[port].dma_channel = dma_claim_unused_channel(true);

    // PIO RX
    uint offset_rx = pio_add_program(ports[port].pio, &pd_receiver_program);
    pio_sm_config c_rx = pd_receiver_program_get_default_config(offset_rx);
    sm_config_set_in_pins(&c_rx, port * 2); // CC1 for port 0, CC2 for port 1 - very simplified
    sm_config_set_jmp_pin(&c_rx, port * 2);
    sm_config_set_in_shift(&c_rx, false, true, 32);
    sm_config_set_fifo_join(&c_rx, PIO_FIFO_JOIN_RX);
    pio_sm_init(ports[port].pio, ports[port].sm_rx, offset_rx, &c_rx);
    pio_sm_set_enabled(ports[port].pio, ports[port].sm_rx, true);

    // PIO TX
    uint offset_tx = pio_add_program(ports[port].pio, &pd_transmitter_program);
    pio_sm_config c_tx = pd_transmitter_program_get_default_config(offset_tx);
    sm_config_set_out_pins(&c_tx, port * 2, 1);
    sm_config_set_sideset_pins(&c_tx, port * 2);
    sm_config_set_out_shift(&c_tx, false, true, 32);
    sm_config_set_fifo_join(&c_tx, PIO_FIFO_JOIN_TX);
    pio_sm_init(ports[port].pio, ports[port].sm_tx, offset_tx, &c_tx);
    pio_sm_set_enabled(ports[port].pio, ports[port].sm_tx, true);

    // DMA
    dma_channel_config dma_config = dma_channel_get_default_config(ports[port].dma_channel);
    channel_config_set_read_increment(&dma_config, false);
    channel_config_set_write_increment(&dma_config, true);
    channel_config_set_dreq(&dma_config, pio_get_dreq(ports[port].pio, ports[port].sm_rx, false));
    dma_channel_configure(
        ports[port].dma_channel,
        &dma_config,
        ports[port].dma_buffer,
        &ports[port].pio->rxf[ports[port].sm_rx],
        DMA_BUFFER_SIZE_PER_PORT,
        false
    );

    dma_channel_set_irq0_enabled(ports[port].dma_channel, true);
    irq_set_exclusive_handler(DMA_IRQ_0 + port, dma_irq_handlers[port]);
    irq_set_enabled(DMA_IRQ_0 + port, true);
    dma_channel_start(ports[port].dma_channel);
    pd_phy_decoder_init(&ports[port].decoder);
#endif
    return ports[port].sm_tx;
}

static pd_packet_t packet_buffer[NUM_PORTS];
static volatile bool packet_received[NUM_PORTS];

static void packet_callback(void* context, pd_packet_t* packet) {
    uintptr_t port = (uintptr_t)context;
    memcpy(&packet_buffer[port], packet, sizeof(pd_packet_t));
    packet_received[port] = true;
}

bool hal_get_packet(unsigned int port, struct pd_packet_s* packet) {
    if (ports[port].read_pos != ports[port].write_pos) {
        uint32_t data[1];
        data[0] = ports[port].dma_buffer[ports[port].read_pos];
        ports[port].read_pos = (ports[port].read_pos + 1) % DMA_BUFFER_SIZE_PER_PORT;
        pd_phy_decode_stream(&ports[port].decoder, data, 1, packet_callback, (uintptr_t)port);
    }
    if (packet_received[port]) {
        memcpy(packet, &packet_buffer[port], sizeof(pd_packet_t));
        packet_received[port] = false;
        return true;
    }
    return false;
}

void hal_pio_init(void) {
    // No-op for now
}

void hal_pio_sm_init(unsigned int sm, unsigned int pin) {
#ifndef NATIVE_BUILD
    if (sm < 4) { // pio0
        uint offset = pio_add_program(pio0, &pd_receiver_program);
        pio_sm_config c = pd_receiver_program_get_default_config(offset);
        sm_config_set_in_pins(&c, pin);
        sm_config_set_in_shift(&c, false, true, 32);
        sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_RX);
        sm_config_set_clkdiv(&c, 12.5f);
        pio_sm_init(pio0, sm, offset, &c);
        pio_sm_set_enabled(pio0, sm, true);
    } else { // pio1
        uint offset = pio_add_program(pio1, &pd_transmitter_program);
        pio_sm_config c = pd_transmitter_program_get_default_config(offset);
        sm_config_set_out_pins(&c, pin, 1);
        sm_config_set_sideset_pins(&c, pin);
        sm_config_set_out_shift(&c, false, true, 32);
        sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);
        sm_config_set_clkdiv(&c, 12.5f);
        pio_sm_init(pio1, sm - 4, offset, &c);
        pio_sm_set_enabled(pio1, sm - 4, true);
    }
#endif
}

void hal_pio_sm_put(unsigned int sm, uint32_t data) {
#ifndef NATIVE_BUILD
    if (sm < 4) {
        pio_sm_put_blocking(pio0, sm, data);
    } else {
        pio_sm_put_blocking(pio1, sm - 4, data);
    }
#endif
}

uint32_t hal_pio_sm_get(unsigned int sm) {
#ifndef NATIVE_BUILD
    if (sm < 4) {
        return pio_sm_get_blocking(pio0, sm);
    } else {
        return 0; // Not implemented for transmitter
    }
#else
    return 0;
#endif
}
