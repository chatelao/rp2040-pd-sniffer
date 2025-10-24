/**
 * @file pd_sink.h
 * @brief USB Power Delivery sink.
 */
#ifndef PD_SINK_H
#define PD_SINK_H

#include "pd_library.h"

/**
 * @brief Represents the state of the USB-PD sink.
 */
typedef enum {
    SINK_STATE_INIT,        /**< The initial state. */
    SINK_STATE_READY,       /**< The sink is ready to receive capabilities. */
    SINK_STATE_REQUESTING,  /**< The sink is requesting a power profile. */
} sink_state_t;

/**
 * @brief Represents a USB-PD sink.
 */
typedef struct {
    sink_state_t state;                     /**< The state of the sink. */
    uint32_t message_id_counter;            /**< The message ID counter. */
    uint64_t state_timer;                   /**< A timer for state transitions. */
    unsigned int tx_sm;                     /**< The transmitter state machine. */

    // Desired capabilities
    uint32_t desired_voltage_mv;            /**< The desired voltage in millivolts. */
    uint32_t desired_current_ma;            /**< The desired current in milliamps. */

    // Source capabilities
    pd_packet_t source_capabilities;        /**< The source capabilities. */

    // Active power profile
    int active_voltage_mv;                  /**< The active voltage in millivolts. */
    int active_current_ma;                  /**< The active current in milliamps. */

    // Requested power profile
    int requested_object_position;          /**< The requested object position. */
} pd_sink_t;

/**
 * @brief Initializes a USB-PD sink.
 * @param sink A pointer to the sink to initialize.
 * @param sm The transmitter state machine to use.
 */
void sink_init(pd_sink_t* sink, unsigned int sm);

/**
 * @brief Ticks a USB-PD sink.
 * @param sink A pointer to the sink to tick.
 */
void sink_tick(pd_sink_t* sink);

/**
 * @brief Handles a USB-PD packet for a sink.
 * @param sink A pointer to the sink to handle the packet for.
 * @param packet The packet to handle.
 */
void sink_handle_packet(pd_sink_t* sink, const pd_packet_t* packet);

/**
 * @brief Requests a power profile from a USB-PD source.
 * @param sink A pointer to the sink to request the power profile for.
 * @param voltage_mv The desired voltage in millivolts.
 * @param current_ma The desired current in milliamps.
 */
void sink_request_power(pd_sink_t* sink, uint32_t voltage_mv, uint32_t current_ma);

#endif // PD_SINK_H
