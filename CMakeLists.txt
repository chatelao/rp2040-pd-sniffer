# Mindestversion von CMake festlegen (3.13 ist Standard für Pico)
cmake_minimum_required(VERSION 3.13)

# Pfad zum Pico SDK hinzufügen
# (Dies sollte den Pfad aus Ihrer Umgebungsvariable PICO_SDK_PATH verwenden)
include($ENV{PICO_SDK_PATH}/pico_sdk_init.cmake)

# Projektname festlegen
project(rp2040_pd_sniffer C CXX ASM)

# Überprüfen Sie, ob das SDK initialisiert wurde und die Zielhardware existiert
if (PICO_SDK_INIT_SUCCESS)
    # Globale Build-Einstellungen hinzufügen
    pico_sdk_init()

    # Zielname festlegen (wird der Name Ihrer .elf/uf2-Datei)
    set(PROJECT_NAME rp2040_pd_sniffer)

    # Hier Ihre Quelldateien definieren
    # Ersetzen Sie main.c, pd_sniffer.c, etc. durch die tatsächlichen Dateien in Ihrem Projekt.
    # Wenn Ihre Quelldateien in einem Unterverzeichnis (z. B. 'src') liegen, verwenden Sie:
    # file(GLOB PROJECT_SOURCES src/*.c src/*.cpp)
    file(GLOB PROJECT_SOURCES
        main.c
        pd_sniffer.c
        hardware_interface.c
    )

    # Executable erstellen
    add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

    # Hier die Pico-Board-Konfiguration übergeben, damit das Build-System weiß, welche Board-Definitionen zu verwenden sind
    # (Diese Variable wird bereits in Ihrem Workflow mit -DPICO_BOARD=... gesetzt, 
    # aber es ist eine gute Praxis, eine Standardeinstellung zu haben)
    if (NOT PICO_BOARD)
        set(PICO_BOARD seeed_xiao_rp2040 CACHE STRING "Board type")
    endif()

    # Standardbibliotheken und das SDK-Target verlinken
    target_link_libraries(${PROJECT_NAME}
        pico_stdlib
        # Fügen Sie hier alle weiteren Bibliotheken hinzu, z. B. USB, etc.
    )

    # Zielgruppeneigenschaften festlegen (für Pico-spezifische Ausgaben wie .uf2)
    pico_add_extra_outputs(${PROJECT_NAME})
    
    # Debug-Optionen hinzufügen (optional, aber empfohlen)
    pico_enable_standard_output(${PROJECT_NAME})
    pico_enable_standard_output_usb(${PROJECT_NAME}) 
endif()
