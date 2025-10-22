#include "unity.h"
#include "hal/hal.h"

extern uint32_t dma_buffer[]; // This is a bit of a hack, but it's the easiest way to access the buffer
extern void hal_dma_irq_handler_mock(void);

void test_hal_dma_circular_buffer(void) {
    hal_dma_init();

    // Simulate a DMA transfer of 10 words
    for (int i = 0; i < 10; i++) {
        dma_buffer[i] = i;
        hal_dma_irq_handler_mock();
    }

    // Read the data from the circular buffer
    uint32_t captured_data[10];
    uint32_t data_len = hal_dma_read(captured_data, 10);

    TEST_ASSERT_EQUAL(10, data_len);
    for (int i = 0; i < 10; i++) {
        TEST_ASSERT_EQUAL(i, captured_data[i]);
    }
}
