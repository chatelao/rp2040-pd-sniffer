#include "pd_sink.h"
#include "hal/hal.h"
#include <string.h>

#ifndef NATIVE_BUILD
#include "pico/time.h"
#endif

void sink_init(pd_sink_t* sink, unsigned int sm) {
    sink->state = SINK_STATE_INIT;
    sink->message_id_counter = 0;
    sink->state_timer = 0;
    sink->tx_sm = sm;
    sink->desired_voltage_mv = 0;
    sink->desired_current_ma = 0;
    sink->active_voltage_mv = 5000;
    sink->active_current_ma = 900;
    sink->requested_object_position = -1;
    memset(&sink->source_capabilities, 0, sizeof(sink->source_capabilities));
}

void sink_request_power(pd_sink_t* sink, uint32_t voltage_mv, uint32_t current_ma) {
    sink->desired_voltage_mv = voltage_mv;
    sink->desired_current_ma = current_ma;
}

/**
 * @brief Sends a GoodCRC message.
 * @param sink A pointer to the sink to send the message for.
 * @param message_id The message ID of the message to acknowledge.
 */
static void send_good_crc(pd_sink_t* sink, int message_id) {
    pd_packet_t packet;
    packet.header = pd_header_build(0, 0x1, false, false, 2, message_id);
    packet.num_data_objects = 0;
    pd_transmit_packet(sink->tx_sm, &packet);
}

/**
 * @brief Sends a Request message.
 * @param sink A pointer to the sink to send the message for.
 * @param object_position The position of the desired PDO.
 * @param operating_current_ma The desired operating current in milliamps.
 */
static void send_request(pd_sink_t* sink, int object_position, uint32_t operating_current_ma) {
    pd_packet_t packet;
    packet.header = pd_header_build(1, 0x2, false, false, 2, sink->message_id_counter);
    packet.num_data_objects = 1;

    packet.data[0] = (object_position << 28) | (1 << 27) | (0 << 25) | (0 << 24) |
                     ((operating_current_ma / 10) << 10) | ((operating_current_ma / 10) << 0);
    pd_transmit_packet(sink->tx_sm, &packet);

    sink->requested_object_position = object_position;
    sink->message_id_counter = (sink->message_id_counter + 1) % 8;
    sink->state = SINK_STATE_REQUESTING;
#ifndef NATIVE_BUILD
    sink->state_timer = time_us_64();
#endif
}

void sink_tick(pd_sink_t* sink) {
    if (sink->state == SINK_STATE_INIT) {
        sink->state = SINK_STATE_READY;
    }

    if (sink->state == SINK_STATE_REQUESTING) {
        if (time_us_64() - sink->state_timer > 500000) {
            sink->state = SINK_STATE_READY;
        }
    }
}

void sink_handle_packet(pd_sink_t* sink, const pd_packet_t* packet) {
    uint16_t message_type = packet->header & 0x1F;
    uint8_t message_id = (packet->header >> 9) & 0x7;

    if (sink->state == SINK_STATE_READY) {
        if (message_type == 0x1) { // Source_Capabilities
            send_good_crc(sink, message_id);
            sink->source_capabilities = *packet;

            if (sink->desired_voltage_mv > 0) {
                int best_object_position = -1;
                uint32_t best_voltage_mv = 0xffffffff;
                uint32_t best_current_ma = 0xffffffff;

                for (int i = 0; i < packet->num_data_objects; ++i) {
                    uint32_t pdo = packet->data[i];
                    if ((pdo >> 30) == 0) { // Fixed supply
                        uint32_t voltage_mv = ((pdo >> 10) & 0x3FF) * 50;
                        uint32_t current_ma = (pdo & 0x3FF) * 10;

                        if (voltage_mv >= sink->desired_voltage_mv && current_ma >= sink->desired_current_ma) {
                            if (best_object_position == -1 ||
                                (voltage_mv < best_voltage_mv) ||
                                (voltage_mv == best_voltage_mv && current_ma < best_current_ma)) {
                                best_object_position = i + 1;
                                best_voltage_mv = voltage_mv;
                                best_current_ma = current_ma;
                            }
                        }
                    }
                }

                if (best_object_position != -1) {
                    send_request(sink, best_object_position, sink->desired_current_ma);
                }
            }
        }
    } else if (sink->state == SINK_STATE_REQUESTING) {
        if (message_type == 0x2) { // Accept
            send_good_crc(sink, message_id);
            if (sink->requested_object_position != -1) {
                uint32_t pdo = sink->source_capabilities.data[sink->requested_object_position - 1];
                sink->active_voltage_mv = ((pdo >> 10) & 0x3FF) * 50;
                sink->active_current_ma = (pdo & 0x3FF) * 10;
            }
            sink->state = SINK_STATE_READY;
        } else if (message_type == 0x3) { // Reject
            send_good_crc(sink, message_id);
            sink->state = SINK_STATE_READY;
        }
    }
}
