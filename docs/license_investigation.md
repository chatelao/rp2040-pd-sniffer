# License Investigation: usb-pd-arduino

## Summary

This document outlines the findings of an investigation into the `usb-pd-arduino` project, located at https://github.com/manuelbl/usb-pd-arduino. The purpose of this investigation was to determine if the project's license is compatible with our own and if there is any code that could be reused.

## License Compatibility

Our project is licensed under the **GNU Affero General Public License v3.0 or later (AGPL-3.0-or-later)**. The `usb-pd-arduino` project is licensed under the **MIT License**.

The MIT license is a permissive license that is compatible with the AGPL-3.0. We can incorporate code from the `usb-pd-arduino` project into our own, provided that we include the original MIT license notice with the code.

## Code Reuse

The `usb-pd-arduino` project is a well-structured and feature-rich implementation of a USB Power Delivery stack for Arduino. However, it is written in C++ and is tightly integrated with the Arduino framework.

Our project is written in C and is designed to be portable across multiple microcontroller platforms. As a result, the code from the `usb-pd-arduino` project cannot be directly reused in our project.

However, the `usb-pd-arduino` project can serve as a valuable reference for our own implementation. The following files are of particular interest:

*   `src/PDSink.h` and `src/PDSink.cpp`: These files implement the sink functionality of the USB-PD stack.
*   `src/PDController.h` and `src/PDController.cpp`: These files implement the core of the USB-PD protocol.

By studying these files, we can gain a better understanding of the USB-PD protocol and how to implement it in our own project.
