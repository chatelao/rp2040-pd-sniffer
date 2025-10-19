# RP2040 USB-PD Sniffer

This project is a passive USB Power Delivery (USB-PD) sniffer that runs on a Raspberry Pi Pico or any other RP2040-based board. It listens to the communication on a USB-C CC (Configuration Channel) line, decodes the messages in real-time, and prints them to the USB serial console.

It's a powerful and cost-effective tool for anyone developing or debugging USB-PD devices.

## Features

*   **PIO-Based Sampling:** Uses the RP2040's Programmable I/O (PIO) to precisely sample the CC line at 2.4 MHz (8x oversampling of the 300 kHz USB-PD bitrate).
*   **Real-Time Decoding:** Performs full, real-time decoding of the physical layer protocol, including:
    *   Biphase Mark Coding (BMC)
    *   4b/5b Symbol Translation
    *   Packet reconstruction (SOP, Header, Data Objects, CRC, EOP)
*   **Self-Contained Build:** The project is configured with a modern CMake build system and includes the Raspberry Pi Pico SDK as a submodule, making it easy to compile.
*   **Automated Builds:** A GitHub Actions workflow automatically builds the firmware on every push to the `main` branch and creates a GitHub Release with the compiled `.uf2` file for easy flashing.

## How It Works

The sniffer hardware (schematics available in the `hw/` directory) uses a simple comparator circuit to convert the analog signal on the CC line into a digital signal that can be read by a GPIO pin on the RP2040.

1.  A PIO state machine is configured to read this GPIO pin at a constant, high-frequency rate.
2.  Direct Memory Access (DMA) is used to continuously transfer the raw sample data from the PIO's FIFO buffer into a large ring buffer in the RP2040's RAM.
3.  The main application core continuously processes the data from this ring buffer.
4.  A software-based decoder translates the raw, oversampled signal into a clean bitstream by implementing a BMC decoder.
5.  A packet-level state machine consumes the bitstream, identifies the packet structure (preamble, SOP, etc.), decodes the 4b/5b symbols, and reconstructs the full USB-PD packet.
6.  Once a complete and valid packet is decoded, its contents (Header, Data Objects, CRC) are printed to the USB serial console.

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
    This is a critical step to download the required Pico SDK.
    ```bash
    git submodule update --init --recursive
    ```

3.  **Run the Build:**
    This will create a `build` directory, configure the project with CMake, and compile the firmware.
    ```bash
    cmake -B build -S .
    cmake --build build --parallel
    ```

4.  **Find the Firmware:**
    If the build is successful, you will find the flashable `.uf2` file inside the `build` directory:
    `build/rp2040_pd_sniffer.uf2`

## Flashing and Usage

1.  **Enter BOOTSEL Mode:** Hold down the `BOOTSEL` button on your Pico and connect it to your computer via USB. It will appear as a removable drive.
2.  **Flash the Firmware:** Drag and drop the `rp2040_pd_sniffer.uf2` file onto the Pico's drive. The Pico will automatically reboot.
3.  **Monitor the Output:** Open a serial monitor (like `minicom`, `putty`, or the Arduino IDE's Serial Monitor) and connect to the Pico's virtual COM port.

You will now see the decoded USB-PD messages printed to the console in real-time as they occur on the CC line you are monitoring.
