#include "i2c_peripheral.h"
#include <stdint.h>
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "pico/i2c_slave.h"
#include "ringbuf.h"


static i2c_peripheral_config_t current_config;
static uint8_t rx_pool[256], tx_pool[256];
static ringbuffer_t rx, tx;

/**
 * Process an I2C slave interrupt
*/
static void i2c_slave_request_handler(i2c_inst_t *i2c, i2c_slave_event_t evt) {
    uint8_t e;

    switch(evt) {
        case I2C_SLAVE_RECEIVE:
            e = i2c_read_byte_raw(i2c);
            ringbuffer_push(&rx, e);
            break;

        case I2C_SLAVE_REQUEST:
            if(!ringbuffer_is_empty(&tx)) {
                e = ringbuffer_pop(&tx);
                i2c_write_byte_raw(i2c, e);
            }
            break;

        case I2C_SLAVE_FINISH:
            break;

        default:
            break;
    }
}

void i2c_peripheral_setup(i2c_peripheral_config_t *config) {
    current_config = *config;

    gpio_init(current_config.sda_pin);
    gpio_init(current_config.scl_pin);

    gpio_set_function(current_config.sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(current_config.scl_pin, GPIO_FUNC_I2C);

    if(current_config.use_internal_pullup) {
        gpio_pull_up(current_config.sda_pin);
        gpio_pull_up(current_config.scl_pin);
    }

    rx = ringbuffer_create(rx_pool, 256);
    tx = ringbuffer_create(tx_pool, 256);

    i2c_init(current_config.controller, current_config.speed);
    i2c_slave_init(i2c1, current_config.address, &i2c_slave_request_handler);
}
