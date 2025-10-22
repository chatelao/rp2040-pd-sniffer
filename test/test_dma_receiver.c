#ifdef NATIVE_BUILD

#include "unity.h"
#include "dma_receiver.h"
#include <stdbool.h>

// Stubs for hardware-specific functions
void dma_channel_set_irq0_enabled(unsigned int channel, bool en) {}
void irq_set_exclusive_handler(unsigned int irq_num, void (*handler)(void)) {}
void irq_set_enabled(unsigned int irq_num, bool enabled) {}
void dma_channel_start(unsigned int channel) {}


void test_dma_receiver_circular_buffer(void) {
    // This is a placeholder for the actual test
    TEST_ASSERT_EQUAL(0, 0);
}

#endif // NATIVE_BUILD
