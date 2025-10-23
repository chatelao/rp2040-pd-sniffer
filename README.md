# RP2040 USB-PD Sniffer and Sink

This project provides two firmwares for a Raspberry Pi Pico or any other RP2040-based board to interact with the USB Power Delivery (USB-PD) protocol.

1.  **Sniffer:** A passive USB-PD sniffer that listens to the communication on a USB-C CC (Configuration Channel) line, decodes the messages in real-time, and prints them to the USB serial console.
2.  **Sink:** An active device that can negotiate a specific power profile from a USB-PD power adapter. This example is configured to request a 15V / 3A profile.

## Features

*   **Shared USB-PD Library:** All the common logic for decoding and encoding USB-PD messages is contained in a shared library.
*   **PIO-Based Sampling and Transmission:** Uses the RP2040's Programmable I/O (PIO) for both an 8x oversampled receiver (sniffer and sink) and a BMC transmitter (sink).
*   **Real-Time Decoding:** Performs full, real-time decoding of the physical layer protocol.
*   **Self-Contained Build:** The project is configured with a modern CMake build system and includes the Raspberry Pi Pico SDK as a submodule.
*   **Automated Builds:** A GitHub Actions workflow automatically builds both firmware targets on every push to the `main` branch.

## Building the Firmware

This project uses CMake and requires the ARM GCC toolchain.

### Prerequisites

*   `git`
*   `cmake`
*   `gcc-arm-none-eabi`

On a Debian-based system (like Ubuntu), you can install these with:
```bash
sudo apt-get update
sudo apt-get install -y git cmake gcc-arm-none-eabi
```

### Build Steps

1.  **Clone the repository:**
    ```bash
    git clone https://github.com/your-username/rp2040-pd-sniffer.git
    cd rp2040-pd-sniffer
    ```

2.  **Initialize the Submodules:**
    ```bash
    git submodule update --init --recursive
    ```

3.  **Run the Build:**
    ```bash
    cmake -B build -S .
    cmake --build build --parallel
    ```

4.  **Find the Firmware:**
    If the build is successful, you will find the flashable `.uf2` files inside the `build` directory:
    *   `build/sniffer.uf2`
    *   `build/sink.uf2`

## Flashing and Usage

### Sniffer

1.  **Flash the Firmware:** Flash `sniffer.uf2` to your Pico.
2.  **Monitor the Output:** Open a serial monitor and connect to the Pico's virtual COM port. You will see the decoded USB-PD messages.

### Sink

1.  **Flash the Firmware:** Flash `sink.uf2` to your Pico.
2.  **Connect to Power Adapter:** Connect the CC line of your circuit to a USB-PD power adapter.
3.  **Monitor the Output:** Open a serial monitor. The application will print a message if it finds and requests the 15V/3A profile.

## Acknowledgements

The development of the flexible, data-driven sink implementation was significantly influenced by the work of Manuel Bleichenbacher on the [usb-pd-arduino](https://github.com/manuelbl/usb-pd-arduino) project. His project served as an invaluable reference for understanding the USB-PD protocol and implementing a robust state machine.

The `usb-pd-arduino` project is licensed under the MIT License, and its license is included below:

```
MIT License

Copyright (c) 2023 Manuel Bleichenbacher

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```
