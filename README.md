# esp32-idle

Minimal ESP-IDF firmware for a classic ESP32 that's debugged over JTAG with
the [esp32c5-jtag](../esp32c5-jtag) adapter. It boots and prints
`esp32-idle: alive N` every 5 s. It leaves GPIO12-15 (MTDI/MTCK/MTMS/MTDO)
alone, so the JTAG pads stay usable.

Use it to replace firmware that takes over the JTAG pins. CircuitPython does
that: it resets all GPIOs at startup. So does anything that uses HSPI on its
default pins 12-15. With that kind of firmware, OpenOCD only sees the chip
briefly after reset and then reads "all ones".

## Build

    . ~/.espressif/tools/activate_idf_v6.0.2.sh
    idf.py set-target esp32    # first time only
    idf.py build

## Flash

Over JTAG, through the C5 adapter:

    openocd -f ../esp32c5-jtag/esp_usb_jtag.ocd -c "adapter speed 6000" -f target/esp32.cfg \
        -c "program_esp build/esp32-idle.bin 0x10000 verify reset exit"

Over the C5's UART pass-through. Use this for a board with a broken USB port,
or when JTAG doesn't work yet. Wiring: C5 GPIO23 -> ESP32 GPIO3 (RX0), C5
GPIO24 <- ESP32 GPIO1 (TX0), GND. Put the ESP32 in download mode (hold BOOT,
tap EN, release BOOT), then:

    cd build
    esptool --chip esp32 -p /dev/serial/by-id/usb-Espressif_USB_JTAG_serial_debug_unit_*-if00 \
        --before no-reset --after no-reset --no-stub write-flash @flash_args

Then press EN. `--no-stub` is needed: esptool sees the C5's USB IDs, treats
the link as native USB, and the stub upload over the 115200 baud UART fails.

The C5 has no line to the ESP32's EN or BOOT pins, so use the buttons for
resets that change the boot mode. A JTAG reset is a software reset and keeps
the boot mode latched at the last EN press.

## Debug with GDB

Start OpenOCD in one terminal and leave it running:

    openocd -f ../esp32c5-jtag/esp_usb_jtag.ocd -c "adapter speed 6000" -f target/esp32.cfg

In another terminal, from this directory:

    xtensa-esp32-elf-gdb -ex "target extended-remote :3333" -ex "monitor reset halt" build/esp32-idle.elf
    (gdb) break main.c:14      # the printf in the loop
    (gdb) continue
    (gdb) bt
    (gdb) info locals
    (gdb) set var n = 1000      # change variables on the chip
    (gdb) info threads          # FreeRTOS tasks, with backtraces
    (gdb) monitor resume
    (gdb) quit

Breakpoints, variable writes and FreeRTOS thread awareness all work through
the C5 adapter.

**End a session with `monitor resume`.** If GDB quits or detaches while the
ESP32 is halted, the chip stays halted afterwards and the firmware just stops,
with no message. Stop OpenOCD with Ctrl+C. Killing it right after GDB detaches
can also leave the chip halted. If it does get stuck, attach GDB and run
`monitor resume`, or press EN.
