/**
 * Blinks a led on GPIO22 every 200ms, and stops if a button in pull-down is
 * pressed on GPIO21.
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

int main() {

    stdio_init_all();

    gpio_init(22);
    gpio_set_dir(22, GPIO_OUT);

    gpio_init(21);
    gpio_set_dir(21, GPIO_IN);


    bool pin_status = true;

    while(true) {
        gpio_put(22, pin_status);

        if(gpio_get(21) == 0) {
            //pressed
            puts("Pressed!");
        } else {
            pin_status ^= true;
        }

        sleep_ms(200);
    }


    return 0;
}