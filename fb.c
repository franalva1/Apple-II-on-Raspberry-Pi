#include "fb.h"
#include "assert.h"
#include "mailbox.h"
#include "printf.h"


typedef struct {
    unsigned int width;       // width of the physical screen
    unsigned int height;      // height of the physical screen
    unsigned int virtual_width;  // width of the virtual framebuffer
    unsigned int virtual_height; // height of the virtual framebuffer
    unsigned int pitch;       // number of bytes per row
    unsigned int bit_depth;   // number of bits per pixel
    unsigned int x_offset;    // x of the upper left corner of the virtual fb
    unsigned int y_offset;    // y of the upper left corner of the virtual fb
    void *framebuffer;        // pointer to the start of the framebuffer
    unsigned int total_bytes; // total number of bytes in the framebuffer
} fb_config_t;

// fb is volatile because the GPU will write to it
static volatile fb_config_t fb __attribute__ ((aligned(16)));

void fb_init(unsigned int width, unsigned int height, unsigned int depth_in_bytes, fb_mode_t mode)
{
    // TODO: extend this function to support double-buffered mode

    fb.width = width;
    fb.virtual_width = width;
    fb.height = height;
    
    if (mode == FB_DOUBLEBUFFER)
	fb.virtual_height = 2*height;
    else 
	fb.virtual_height = height;
    fb.bit_depth = depth_in_bytes * 8; // convert number of bytes to number of bits
    fb.x_offset = 0;
    fb.y_offset = 0;

    // the manual states that we must set these value to 0
    // the GPU will return new values in its response
    fb.pitch = 0;
    fb.framebuffer = 0;
    fb.total_bytes = 0;

    // Send address of fb struct to the GPU as message
    bool mailbox_success = mailbox_request(MAILBOX_FRAMEBUFFER, (unsigned int)&fb);
    assert(mailbox_success); // confirm successful config
}


void fb_swap_buffer(void)
{
    
    if (fb.virtual_height == fb.height)
	return;

    // change which framebuffer is on screen
    // if we are currently on the top fb
    if (fb.y_offset == 0) 
	fb.y_offset = fb.height;
    else 
	fb.y_offset = 0;

    // inform GPU of the change in y offset
    bool mailbox_success = mailbox_request(MAILBOX_FRAMEBUFFER, (unsigned int)&fb);
    assert(mailbox_success);

}

void* fb_get_draw_buffer(void)
{
    // declare the fb as a 2D array:
    //
    //	   row1: pix1, pix2, pix3..... 
    //     row2: 
    //	    ...
    //

    int (*newFB)[fb_get_pitch()/(fb.bit_depth/8)] = fb.framebuffer;
    
    if (fb.virtual_height != fb.height) {

	if (fb.y_offset == 0) {
	    return (void *)newFB[fb.height];
	} 

    }

    return (void *)newFB;

}

unsigned int fb_get_width(void)
{
    return fb.width;
}

unsigned int fb_get_height(void)
{
    return fb.height;
}

unsigned int fb_get_depth(void)
{
    return fb.bit_depth/8;
}

unsigned int fb_get_pitch(void)
{
    return fb.pitch;
}

