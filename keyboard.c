#include "keyboard.h"
#include "ps2.h"
#include "printf.h"

static ps2_device_t *dev;
static unsigned char mods = 0;

void keyboard_init(unsigned int clock_gpio, unsigned int data_gpio)
{
    dev = ps2_new(clock_gpio, data_gpio);
}

unsigned char keyboard_read_scancode(void)
{
    return ps2_read(dev);
}

key_action_t keyboard_read_sequence(void)
{
    key_action_t action = { 0 };

    unsigned char keyVal = keyboard_read_scancode();
  
    // if its' an extended key 
    if (keyVal == 0xE0 ){
	keyVal = keyboard_read_scancode();
	
	// if its' an extended key break
	if (keyVal == 0xF0) {
	    action.what = KEY_RELEASE;
	    action.keycode = keyboard_read_scancode();
	  
	// if its' an extended key press
	} else {
	    action.what = KEY_PRESS;
	    action.keycode = keyVal;
	}

    // if its' a break code
    } else if (keyVal == 0xF0) {
	action.what = KEY_RELEASE;
	action.keycode = keyboard_read_scancode();
	
    // if its a normal key
    } else {
	action.what = KEY_PRESS;
	action.keycode = keyVal;
    }

    // return our action
    return action;
}

key_event_t keyboard_read_event(void)
{
    key_event_t event = { 0 };
    
    // get the action struct

    key_action_t action;
    unsigned char keyVal;
    

    // run this loop until a non special key is pressed 
    while (1) {

	// action to be read
	action = keyboard_read_sequence();
    	
	// the key that this action pertains to
	keyVal = action.keycode;

	// if shift was actioned
	if (keyVal == 0x12 || keyVal == 0x59) {

		// if it was pressed, adjust the mod
		if (action.what == KEY_PRESS) {
			mods |= 0b1;

		// if it was released, adjust the mod
		} else {
			mods &= 0b1110;
		
		}
	// if alt was actionedd
	} else if (keyVal == 0x11) {

		// if it was pressed, adjust the mod
		if (action.what == KEY_PRESS) {
			mods |= 0b10;
		
		// if it was released, adjust the mod
		} else {		
			mods &= 0b1101;
	
		}
	// if ctrl was actioned
	} else if (keyVal == 0x14) {

		// if it was pressed, adjust the mod
		if (action.what == KEY_PRESS) {
			mods |= 0b100;

		// if it was released, adjust the mod
		} else {
			mods &= 0b1011;

		}
	// if capslock was actioned
	} else if (keyVal == 0x58) {

		// if it was pressed, see mod
		if (action.what == KEY_PRESS) {
		
			// if its' on, turn it off
		    	if (mods>=8) 
				mods &= 0b0111;
			else // if its off, turn it on
				mods |= 0b1000;
		}

	// if the key pressed wasn't a special key, leave the loop
	} else {
		break;
	}
    }

    // now since its' a normal key

    event.action = action;

    event.key = ps2_keys[keyVal];

    event.modifiers = mods;

    return event;
}

unsigned char keyboard_read_next(void)
{
   
    key_event_t event = keyboard_read_event();

    // wait for a key press
    while (1) {
    	if (event.action.what == KEY_PRESS) 
		break;
	event = keyboard_read_event();
    }


    unsigned char output;

    // if both the shift and caps lock modifier is present or just shift
    if (mods == (KEYBOARD_MOD_CAPS_LOCK + KEYBOARD_MOD_SHIFT) || 
	mods == KEYBOARD_MOD_SHIFT) {
	return event.key.other_ch;

    // if only caps lock is present 
    } else if (mods == KEYBOARD_MOD_CAPS_LOCK) {
	
	// store the ascii value of the key pressed in output
	output = event.key.ch;
	
	// only if its' a letter, return the capital version
	if (output >= 0x61 && output <= 0x7A) {
	    return event.key.other_ch;
	
	// if its' not a letter key, return the normal version
	} else {
	    return output;
	}
    }

    output = event.key.ch;

    return output;
}
