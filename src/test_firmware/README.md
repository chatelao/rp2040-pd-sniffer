# Hardware-in-the-Loop Testing

This directory contains a set of firmware applications for performing hardware-in-the-loop (HIL) testing of the USB-PD sniffer and sink firmware. These tests are designed to be run with two Raspberry Pi Pico boards connected to each other.

## Hardware Setup

You will need two Raspberry Pi Pico boards and three jumper wires. Connect the boards as follows:

*   **Pico 1, GP28 (CC1) <-> Pico 2, GP28 (CC1)**
*   **Pico 1, GP29 (CC2) <-> Pico 2, GP29 (CC2)**
*   **Pico 1, GND <-> Pico 2, GND**

A visual representation of the wiring:

```
+-----------------+        +-----------------+
|      Pico 1     |        |      Pico 2     |
|                 |        |                 |
|            GP28 | ------ | GP28            |
|            GP29 | ------ | GP29            |
|             GND | ------ | GND             |
|                 |        |                 |
+-----------------+        +-----------------+
```

## Ping-Pong Test

This test validates the basic transmission and reception capabilities of the `pd_library`.

1.  **Flash Firmware:**
    *   Flash `pinger.uf2` onto Pico 1.
    *   Flash `ponger.uf2` onto Pico 2.
2.  **Run Test:**
    *   Connect both Picos to your computer via USB to power them and open a serial monitor for each.
    *   The `pinger` will begin sending "PING" messages on CC1.
    *   The `ponger` will listen for these messages on CC1 and, upon receiving one, will send a "PONG" message back on CC1.
3.  **Expected Outcome:**
    *   The onboard LED on the `pinger` board will toggle every time it receives a "PONG" message.
    *   The onboard LED on the `ponger` board will toggle every time it receives a "PING" message.
    *   You will see corresponding "PING received" and "PONG received" messages in the serial monitors.

## Source/Sink Negotiation Test

This test validates a more realistic USB-PD negotiation scenario.

1.  **Flash Firmware:**
    *   Flash `source_emulator.uf2` onto Pico 1.
    *   Flash `sink_tester.uf2` onto Pico 2.
2.  **Run Test:**
    *   Connect both Picos to your computer via USB and open a serial monitor for each.
    *   The `source_emulator` will begin broadcasting `Source_Capabilities` messages on CC1, advertising a 5V/3A profile.
    *   The `sink_tester` will listen for these messages on CC1 and, upon receiving one, will send a `Request` message back.
3.  **Expected Outcome:**
    *   The onboard LED on the `source_emulator` board will toggle every time it broadcasts its capabilities.
    *   The onboard LED on the `sink_tester` board will toggle every time it receives a valid `Source_Capabilities` message and sends a `Request`.
    *   You will see corresponding messages in the serial monitors indicating that the negotiation is taking place.
