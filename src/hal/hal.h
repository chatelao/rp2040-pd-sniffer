#ifndef HAL_H
#define HAL_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

struct pd_packet_s;

// GPIO
void hal_gpio_init(unsigned int pin);
void hal_gpio_set_dir(unsigned int pin, bool out);
void hal_gpio_put(unsigned int pin, bool value);

// DMA
void hal_dma_init(void);
void hal_dma_start_transfer(uint32_t* src, uint32_t* dest, size_t len);
uint32_t hal_dma_read(uint32_t* buffer, uint32_t len);

// CRC
uint32_t hal_crc32(const uint8_t *data, size_t len);

// PIO / State Machine
void hal_pio_init(void);
void hal_pio_sm_init(unsigned int sm, unsigned int pin);
void hal_pio_sm_put(unsigned int sm, uint32_t data);
uint32_t hal_pio_sm_get(unsigned int sm);

// High level PD functions
unsigned int hal_init(unsigned int port);
bool hal_get_packet(unsigned int port, struct pd_packet_s* packet);

#endif // HAL_H
