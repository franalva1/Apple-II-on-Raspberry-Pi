#include "gpio.h"

void gpio_init(void) {
    // no initialization required for this peripheral
}

void gpio_set_function(unsigned int pin, unsigned int function) {
   
   // checking for invalid inputs 
   
   if (pin < GPIO_PIN_FIRST || pin > GPIO_PIN_LAST || function < GPIO_FUNC_INPUT || function > GPIO_FUNC_ALT3)
	return;
   
   //now we do what we need to do

   volatile unsigned int * theOneToSet = (unsigned int *) 0x20200000;

   if (pin > 9)
	theOneToSet += ((int)(pin/10));
   
   *theOneToSet &= ~(0x7 << ((pin%10)*3));
   *theOneToSet |= (function << ((pin%10)*3));
	
}

unsigned int gpio_get_function(unsigned int pin) {
    
   if (pin < GPIO_PIN_FIRST || pin > GPIO_PIN_LAST)  
	return -1;

   volatile unsigned int * theOneToGet = (unsigned int *) 0x20200000;

   if (pin > 9)
        theOneToGet += ((int)(pin/10));

   unsigned int daValue = *theOneToGet;

   daValue &= (0b111 << ((pin%10)*3));
   daValue =  daValue >> ((pin%10)*3);
   return daValue;
}

void gpio_set_input(unsigned int pin) {
    gpio_set_function(pin, GPIO_FUNC_INPUT);
}

void gpio_set_output(unsigned int pin) {
    gpio_set_function(pin, GPIO_FUNC_OUTPUT);
}

void gpio_write(unsigned int pin, unsigned int value) {
   
   // checking for invalid inputs

   if ( pin < GPIO_PIN_FIRST || pin > GPIO_PIN_LAST || value < 0 || value > 1)
	return;

   volatile unsigned int * baseSet = (unsigned int *) 0x2020001C;
   volatile unsigned int * baseClear = (unsigned int *) 0x20200028;

   if (pin>=32) {
	baseSet++;
	baseClear++;
	pin = pin%32;	
   }

   if (value == 1) {
	*baseSet = (0b1 << pin);
   } else {
	*baseClear = (0b1 << pin);
   }

}

// pain


unsigned int gpio_read(unsigned int pin) {

   // checking for invalid inputs

   if ( pin < GPIO_PIN_FIRST )
	return GPIO_INVALID_REQUEST;

   volatile unsigned int * baseLev = (unsigned int *) 0x20200034;

   if (pin >= 32) {
	
	baseLev++;
	pin = pin%32;
	
   }

   unsigned int pos = *baseLev;

   pos &=  (0b1 << pin);

   return (pos >> pin);

}
