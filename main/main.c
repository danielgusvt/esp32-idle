/*
 * Minimal "JTAG target" firmware for a classic ESP32: boots, prints a line
 * every few seconds and otherwise leaves GPIO12-15 (MTDI/MTCK/MTMS/MTDO)
 * alone, so the JTAG pads stay usable. Flash your real application over JTAG
 * with openocd's program_esp when you're ready.
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void)
{
    for (unsigned n = 0;; n++) {
        printf("esp32-idle: alive %u\n", n);
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
