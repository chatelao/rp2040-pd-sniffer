# Agent Instructions for `rp2040-pd-sniffer`

This document provides instructions for AI agents working on this repository.

## Project Overview

This project provides USB-PD tools for the RP2040 microcontroller. It includes a passive 'sniffer', an active 'sink', a 'mitm' (Man-in-the-Middle), a native unit test suite, and Hardware-in-the-Loop (HIL) test firmware. The goal is to create a feature-rich, reliable, and easy-to-use tool for USB-PD analysis and interaction.

## Development Workflow

1.  **Understand the Goal:** Before making changes, understand the user's request and the project's architecture.
2.  **Propose a Plan:** Create a clear, step-by-step plan.
3.  **Implement Incrementally:** Make small, verifiable changes.
4.  **Test Thoroughly:** Add new tests for new features and run existing tests to check for regressions.
5.  **Refactor Aggressively:** The user has requested aggressive refactoring to improve the codebase. Prioritize improvements over maintaining stability for now.

## Code Style

- Use Doxygen-style comments (`/** ... */`) for all public functions, structs, and enums in header files.
- Follow the existing code style (based on the Pico SDK).

## Key Architectural Concepts

- **Hardware Abstraction Layer (HAL):** The HAL in `src/hal` separates platform-agnostic logic from platform-specific code. The HAL provides a port-based abstraction (`hal_init(port)`, `hal_get_packet(port, ...)`), and the Pico implementation (`hal_pico.c`) uses PIO and DMA for interrupt-driven packet reception. To avoid circular dependencies, `hal.h` uses a forward declaration for `struct pd_packet_s`.
- **Shared Library (`pd_library`):** The `src/pd_library` contains all the common logic for decoding and encoding USB-PD messages.
- **Physical Layer (`pd_phy`):** The `pd_phy.c`/`.h` module handles the low-level BMC and 4b/5b decoding. It uses a stateful decoder (`pd_phy_decoder_t`) and a callback mechanism for integration.
- **Stateful, Instance-Based Modules:** Modules like `pd_sink` are instance-based (`pd_sink_t`) to allow multiple instances to run concurrently.

## Testing

- **Native Unit Tests:** A native unit test suite is located in the `test/` directory. To build and run the tests:
    1.  `cmake -B build -S . -DNATIVE_BUILD=ON`
    2.  `cmake --build build`
    3.  `cd build && ctest --output-on-failure`
- **Test-Driven Development (TDD):** When fixing a bug, first create a new test case that fails. After implementing the fix, the new test case should pass, and all existing tests should also pass.

## Documentation

- **Doxygen:** The project uses Doxygen for code documentation. The configuration is in the `Doxyfile`, and the output is in the `docs/` directory.
- **README:** The `README.md` file provides a high-level overview of the project.

## References

- **Doxygen Documentation:** The full API documentation is hosted on GitHub Pages [here](https://chatelao.github.io/rp2040-pd-sniffer/).
- **`usb-pd-arduino`:** The `manuelbl/usb-pd-arduino` repository is considered a "field proved" and "reality tested" reference, particularly for CRC calculations and potential STM32 implementation.

## CI/CD

The GitHub Actions workflow in `.github/workflows/main.yml` automates the following:
- Installing dependencies
- Building the firmware
- Running `cppcheck` for static analysis
- Generating Doxygen documentation
- Deploying documentation to GitHub Pages on pushes to `main`
