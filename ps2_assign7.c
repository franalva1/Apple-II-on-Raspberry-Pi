#include "gpio.h"
#include "gpio_extra.h"
#include "malloc.h"
#include "ps2.h"
#include "gpio_extra.h"
#include "gpio_interrupts.h"
#include "uart.h"
#include "ringbuffer.h"

//
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
    unsigned char code;
    int numread;
    int numOnes;
    rb_t *rb;
};


void resetCode(ps2_device_t * dev) {
    dev->code = 0;
    dev->numread = 0;
    dev->numOnes = 0;
}




void enhanced_read_bit(unsigned int pc, void *aux_data) {
    
    ps2_device_t * dev = ((ps2_device_t *)aux_data);

    gpio_check_and_clear_event(dev->clock);

    int bit = gpio_read(dev->data);

    dev->numread++;

    // checking that the first one is correct;
    if (dev->numread == 1) {
	
	// if incorrect reset the code
	if (bit == 1) 
	    resetCode(dev);
	
	// read next bit
	return;
    }


    // count num ones
    if (bit == 1)
        (dev->numOnes)++;    

    // checking parity bit
    if (dev->numread == 10) {

        // if incorrect reset the code
        if ((dev->numOnes)%2 == 0) 
	    resetCode(dev);

    // checking final bit	
    } else if (dev->numread == 11) {
	
        // if correct, enqueue, if not, reset code
	if (bit == 1)
	    rb_enqueue(dev->rb, dev->code);
   
        resetCode(dev);
    } else {
        int currentCode;
        currentCode = bit << ((dev->numread)-2);
        (dev->code) |= currentCode;
    }
 
}



// Creates a new PS2 device with a particular clock and data pin,
// initializing pins to be pull-up inputs.
ps2_device_t *ps2_new(unsigned int clock_gpio, unsigned int data_gpio)
{
    // consider why must malloc be used to allocate device
    ps2_device_t *dev = malloc(sizeof(*dev));

    dev->clock = clock_gpio;
    
    gpio_interrupts_init();
    gpio_set_input(dev->clock);
    gpio_set_pullup(dev->clock);

    gpio_enable_event_detection(clock_gpio, GPIO_DETECT_FALLING_EDGE);
    gpio_interrupts_register_handler(clock_gpio, enhanced_read_bit, dev);
    gpio_interrupts_enable();


    dev->rb = rb_new();

    dev->data = data_gpio;
    gpio_set_input(dev->data);
    gpio_set_pullup(dev->data);

    resetCode(dev);    

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

    while (rb_empty(dev->rb)) {}

    int scanCode = 0;

    rb_dequeue(dev->rb, &scanCode);

    return (unsigned char)scanCode;

}


// Copy/paste your code from assign5/ps2.c as starting point
