#include "i2c_peripheral.h"
#include <stdint.h>
#include <stdbool.h>
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "pico/i2c_slave.h"
#include "ringbuf.h"


static i2c_peripheral_config_t current_config;
static uint8_t rx_pool[256], tx_pool[256];
static ringbuffer_t rx, tx;
static bool read_requested;     /* < master requested data, but tx buffer was empty */

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

        case I2C_SLAVE_REQUEST:     // fast read, if nothing is enqueued the loop will take care of sending data
            if(!ringbuffer_is_empty(&tx)) {
                e = ringbuffer_pop(&tx);
                i2c_write_byte_raw(i2c, e);
            } else {
                read_requested = true;
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


static uint8_t registerfile[256];

enum fsm_states {
    AWAITING_ADDRESS,
    RECEIVED_ADDRESS,
    SEND_DATA,          // < prepare data in tx buffer and sends the first byte
    RECEIVE_DATA,       // < receive and save data
    TOTAL_STATES
};

void i2c_peripheral_loop() {
    static enum fsm_states state = AWAITING_ADDRESS;
    static int address;
    uint8_t e;

    switch(state) {
        case AWAITING_ADDRESS:
            if(!ringbuffer_is_empty(&rx)) {
                address = ringbuffer_pop(&rx);
                state = RECEIVED_ADDRESS;
            }
            break;
        
        case RECEIVED_ADDRESS:
            if(read_requested) {
                state = SEND_DATA;
            } else {
                state = RECEIVE_DATA;
            }
            break;

        case SEND_DATA:
            //ringbuffer_push(&tx, registerfile[address]);
            i2c_write_byte_raw(current_config.controller, registerfile[address]);
            state = AWAITING_ADDRESS;
            break;

        case RECEIVE_DATA:
            e = ringbuffer_pop(&rx);
            registerfile[address] = e;
            state = AWAITING_ADDRESS;
            break;

        default:
            state = AWAITING_ADDRESS;
            break;
    }
}