/**
 * Prints "Hello world" each second.
 * 
 * Compiling:
 * copy the content of this file into main.c, or edit the CMakeLists.txt in the root
 * directory of this repository and use this helloworld.c file in the `add_executable` command.
*/

#include <stdlib.h>
#include <stdio.h>
#include "pico/stdlib.h"

int main() {

    stdio_init_all();

    while(true) {
        puts("Hello world");

        sleep_ms(1000);
    }

    return 0;
}