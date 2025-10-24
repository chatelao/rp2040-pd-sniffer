/**
 * @file hal.h
 * @brief Hardware Abstraction Layer for USB Power Delivery.
 */
#ifndef HAL_H
#define HAL_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

struct pd_packet_s;

/**
 * @brief Initializes a GPIO pin.
 * @param pin The GPIO pin number.
 */
void hal_gpio_init(unsigned int pin);

/**
 * @brief Sets the direction of a GPIO pin.
 * @param pin The GPIO pin number.
 * @param out True for output, false for input.
 */
void hal_gpio_set_dir(unsigned int pin, bool out);

/**
 * @brief Sets the value of a GPIO pin.
 * @param pin The GPIO pin number.
 * @param value The value to set the pin to.
 */
void hal_gpio_put(unsigned int pin, bool value);

/**
 * @brief Initializes the DMA controller.
 */
void hal_dma_init(void);

/**
 * @brief Starts a DMA transfer.
 * @param src The source address.
 * @param dest The destination address.
 * @param len The number of bytes to transfer.
 */
void hal_dma_start_transfer(uint32_t* src, uint32_t* dest, size_t len);

/**
 * @brief Reads data from the DMA controller.
 * @param buffer The buffer to read data into.
 * @param len The maximum number of bytes to read.
 * @return The number of bytes read.
 */
uint32_t hal_dma_read(uint32_t* buffer, uint32_t len);

/**
 * @brief Calculates the CRC32 of a block of data.
 * @param data The data to calculate the CRC32 of.
 * @param len The length of the data in bytes.
 * @return The CRC32 of the data.
 */
uint32_t hal_crc32(const uint8_t *data, size_t len);

/**
 * @brief Initializes the PIO controller.
 */
void hal_pio_init(void);

/**
 * @brief Initializes a PIO state machine.
 * @param sm The state machine number.
 * @param pin The pin to associate with the state machine.
 */
void hal_pio_sm_init(unsigned int sm, unsigned int pin);

/**
 * @brief Puts a word into a PIO state machine's TX FIFO.
 * @param sm The state machine number.
 * @param data The word to put into the FIFO.
 */
void hal_pio_sm_put(unsigned int sm, uint32_t data);

/**
 * @brief Gets a word from a PIO state machine's RX FIFO.
 * @param sm The state machine number.
 * @return The word from the FIFO.
 */
uint32_t hal_pio_sm_get(unsigned int sm);

/**
 * @brief Initializes a USB-PD port.
 * @param port The port number.
 * @return The transmitter state machine number.
 */
unsigned int hal_init(unsigned int port);

/**
 * @brief Gets a USB-PD packet from a port.
 * @param port The port number.
 * @param packet A pointer to a packet structure to fill.
 * @return True if a packet was received, false otherwise.
 */
bool hal_get_packet(unsigned int port, struct pd_packet_s* packet);

#endif // HAL_H
