# This file defines a custom target that creates a "flash" command
# that will copy the executable on the Pico using UF2 protocol.

# TODO: abstract the pico folder path, and remove the hard-coded one that only works on macOS

add_custom_target(flash
    COMMAND cp ${CMAKE_BINARY_DIR}/*.uf2 /Volumes/RPI-RP2
)