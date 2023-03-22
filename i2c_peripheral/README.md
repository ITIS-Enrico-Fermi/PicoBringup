# i2c_peripheral

This library provides a framework for implementation of I2C peripherals on the Pico.

It's built on top of `hardware_i2c`, `hardware_gpio` and `pico_i2c_slave` and adds two abstraction layers:

- a `streamifier` library that provides a "stream-like" interface to the underlying I2C controller
operating in slave mode;

- a `protocol` module that defines the business logic of the peripheral, in other words how the slave
responds to reads and writes from the master. A sample but rather useful "register-like" function mode
is provided, but the user may define its custom logic.

The library doesn't impose any coding style, but keep in mind that is designed to work in a superloop
environemnt (or PLC-like program) where the function "i2c_peripheral_loop" is called once every few times.
