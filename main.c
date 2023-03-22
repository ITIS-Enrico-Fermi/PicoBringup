#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "pico/i2c_slave.h"

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
void i2c_slave_request_handler(i2c_inst_t *i2c, i2c_slave_event_t evt) {
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

void i2c_slave_loop() {
    if(data_available) {
        putchar(i2c_slave_mempool[mempool_index-1]);
        data_available = 0;
        mempool_index = 0;
    }
}


static uint8_t master_mempool[256];

void i2c_master_loop() {
    static int mode = 0;

    if(mode == 0) {
        int written = i2c_write_timeout_us(i2c0, SLAVE_ADDR, "hellohello", 10, false, 1000000);
        printf("Written: %d\n", written);
        mode = 1;
    } else {
        int readbytes = i2c_read_timeout_us(i2c0, SLAVE_ADDR, master_mempool, 11, false, 1000000);
        if(readbytes > 0) {
            printf("From master: ");
            for(int i=0; i<readbytes; i++)
                putchar(master_mempool[i]);
            putchar('\n');
        }
        mode = 0;
    }
}


int main() {

    stdio_init_all();

    blink_setup();
    button_setup();

    i2c_master_setup();
    i2c_slave_setup();

    while(true) {
        blink_loop();
        //i2c_slave_loop();

        if(gpio_get(21) == 0) {
            //pressed
            puts("Pressed!");
        }

        i2c_master_loop();
        printf("Data requested: %d, data received: %d\n", data_requested, data_available);

        sleep_ms(200);
    }


    return 0;
}