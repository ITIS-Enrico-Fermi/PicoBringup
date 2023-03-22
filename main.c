#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "pico/i2c_slave.h"
#include "i2c_peripheral/i2c_peripheral.h"

void blink_setup() {
    gpio_init(22);
    gpio_set_dir(22, GPIO_OUT);
}

void blink_loop() {
    static bool pin_status = true;

    gpio_put(22, pin_status);
    pin_status ^= true;
}

void button_setup() {
    gpio_init(21);
    gpio_set_dir(21, GPIO_IN);
}


#define I2C_MASTER_SDA  16
#define I2C_MASTER_SCL  17

/**
 * Sets up I2C controller 0 as master
 * on pins GPIO16 and GPIO17 (SDA, SCL)
*/
void i2c_master_setup() {
    gpio_init(I2C_MASTER_SDA);
    gpio_init(I2C_MASTER_SCL);

    gpio_set_function(I2C_MASTER_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_MASTER_SCL, GPIO_FUNC_I2C);

    gpio_pull_up(I2C_MASTER_SDA);
    gpio_pull_up(I2C_MASTER_SCL);

    i2c_init(i2c0, 100000);
}


#define I2C_SLAVE_SDA  14
#define I2C_SLAVE_SCL  15

#define SLAVE_ADDR  0xa0

static uint8_t i2c_slave_mempool[256];
static int mempool_index = 0;
static bool data_available = false;
static bool data_requested = false;

static char *slave_response = "Hello world\n";
#define RESPONSE_LEN 12
static int slave_resp_index = 0;

/**
 * Process an I2C slave interrupt
*/
static void i2c_slave_request_handler(i2c_inst_t *i2c, i2c_slave_event_t evt) {
    switch(evt) {
        case I2C_SLAVE_RECEIVE:
            if(mempool_index < 256) {
                i2c_slave_mempool[mempool_index++] = i2c_read_byte_raw(i2c);
                data_available = true;
            }
            break;
        case I2C_SLAVE_REQUEST:
            i2c_write_byte_raw(i2c, slave_response[slave_resp_index++]);
            if(slave_resp_index == RESPONSE_LEN)
                slave_resp_index = 0;
            data_requested = true;
            break;
        case I2C_SLAVE_FINISH:
            break;
        default:
            break;
    }
}

/**
 * Sets up I2C controller 1 as slave
 * on pins GPIO14 and GPIO15 (SDA, SCL)
*/
void i2c_slave_setup() {
    gpio_init(I2C_SLAVE_SDA);
    gpio_init(I2C_SLAVE_SCL);

    gpio_set_function(I2C_SLAVE_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SLAVE_SCL, GPIO_FUNC_I2C);

    gpio_pull_up(I2C_SLAVE_SDA);
    gpio_pull_up(I2C_SLAVE_SCL);

    i2c_init(i2c1, 100000);
    i2c_slave_init(i2c1, SLAVE_ADDR, &i2c_slave_request_handler);
}


int main() {

    stdio_init_all();

    i2c_master_setup();

    i2c_peripheral_config_t conf = {
        .sda_pin = I2C_SLAVE_SDA,
        .scl_pin = I2C_SLAVE_SCL,
        .speed = 100000,
        .use_internal_pullup = true,
        .controller = i2c1,
        .address = SLAVE_ADDR
    };
    i2c_peripheral_setup(&conf);

    int master_readbytes = 0;
    char read_string[100];

    int mode = 0;
    int i = 0;

    while(true) {
        i2c_peripheral_loop();

        if(mode == 1) {
            master_readbytes = i2c_read_blocking(i2c0, SLAVE_ADDR, read_string, 1, false);

            if(master_readbytes > 0) {
                read_string[master_readbytes] = 0;
                printf("%x", read_string[0]);
            } else {
                printf("Read return code: %d\n", master_readbytes);
            }

            mode = 0;
        } else {
            i2c_write_blocking(i2c0, SLAVE_ADDR, (uint8_t *)&i, 1, false);

            i++;
            if(i == 256)
                i=0;
            mode = 1;
        }
        
        sleep_ms(100);
    }


    return 0;
}