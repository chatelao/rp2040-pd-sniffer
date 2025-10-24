/**
 * @file source_emulator_logic.h
 * @brief Logic for the source emulator.
 */
#ifndef SOURCE_EMULATOR_LOGIC_H
#define SOURCE_EMULATOR_LOGIC_H

#include "pd_library/pd_library.h"

/**
 * @brief Prepares a Source_Capabilities packet.
 * @param packet A pointer to the packet to prepare.
 */
void source_emulator_prepare_packet(pd_packet_t* packet);

#endif // SOURCE_EMULATOR_LOGIC_H
