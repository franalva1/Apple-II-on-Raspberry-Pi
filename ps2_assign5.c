#include "gpio.h"
#include "gpio_extra.h"
#include "malloc.h"
#include "ps2.h"

// A ps2_device is a structure that stores all of the state and information
// needed for a PS2 device. The clock field stores the pin number for the
// clock GPIO pin, and the data field stores the pin number for the data
// GPIO pin. ps2_new shows examples of how these fields are used.
//
// You may extend this structure with additional fields. As
// it is passed into all ps2_ calls, storing state in this structure is
// preferable to using global variables: it allows your driver to
// communicate with more than one concurrent PS2 device (e.g., a keyboard
// and a mouse).
//
// This definition fills out the structure declared in ps2.h.
struct ps2_device {
    unsigned int clock;
    unsigned int data;
};

// Creates a new PS2 device with a particular clock and data pin,
// initializing pins to be pull-up inputs.
ps2_device_t *ps2_new(unsigned int clock_gpio, unsigned int data_gpio)
{
    // consider why must malloc be used to allocate device
    ps2_device_t *dev = malloc(sizeof(*dev));

    dev->clock = clock_gpio;
    gpio_set_input(dev->clock);
    gpio_set_pullup(dev->clock);

    dev->data = data_gpio;
    gpio_set_input(dev->data);
    gpio_set_pullup(dev->data);
    return dev;
}


int read_bit(ps2_device_t *dev) {
    while (gpio_read(dev->clock) == 0) {}
    while (gpio_read(dev->clock) == 1) {}
    return gpio_read(dev->data);
}  



// Read a single PS2 scan code. Always returns a correctly received scan code:
// if an error occurs (e.g., start bit not detected, parity is wrong), the
// function should read another scan code.

unsigned char ps2_read(ps2_device_t *dev)
{

    while(read_bit(dev) != 0) {}

    unsigned char code = 0;

    int currentCode;

    int currentBit;

    int num1s=0;

    for (int i = 0; i<10; i++) {
	currentBit = read_bit(dev);
        
	if (currentBit == 1) num1s++;

	if ( i == 8 && num1s%2 == 0) return ps2_read(dev);
	
	if ( i == 9 &&  currentBit != 1) return ps2_read(dev);

	currentCode = currentBit << i;
        code |= currentCode;

    }

    // TODO: Implement this function during lab5
        // Writing a separate helper function read_bit() is highly
        // recommended: this function waits for a clock falling
        // edge then reads the data pin. Refer to the Keyboard
        // lectures or lab handout on how to wait for a falling edge.
    return code;
}

