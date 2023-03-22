#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "hardware/i2c.h"

typedef struct {
    int             sda_pin;
    int             scl_pin;
    bool            use_internal_pullup;
    int             speed;
    int             address;
    i2c_inst_t *    controller;
} i2c_peripheral_config_t;

/**
 * Setup a new I2C peripheral on the given controller, using the parameters
 * passed in config struct.
*/
void i2c_peripheral_setup(i2c_peripheral_config_t *config);
void i2c_peripheral_loop();