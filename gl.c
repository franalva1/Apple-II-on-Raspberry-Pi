#include "gl.h"
#include "font.h"

#include "printf.h"

void gl_init(unsigned int width, unsigned int height, gl_mode_t mode)
{
    fb_init(width, height, 4, mode);    // use 32-bit depth always for graphics library
}

void gl_swap_buffer(void)
{
    fb_swap_buffer();
}

unsigned int gl_get_width(void)
{
    return fb_get_width();
}

unsigned int gl_get_height(void)
{
    return fb_get_height();
}

color_t gl_color(unsigned char r, unsigned char g, unsigned char b)
{
    color_t result = (0xff << 24) + (r << 16) + (g << 8) + b;
    return result;
}

void gl_clear(color_t c)
{
    // declare the fb as an array of rows   
    int (*fb)[fb_get_pitch()/4] = fb_get_draw_buffer();

    // cycle through each row
    for (int y = 0; y< gl_get_height(); y++){

	// cycle through each colomn
	for (int x = 0; x < fb_get_pitch()/4; x++) {
	    
	    // dereference the pixel and store the given 
	    // color in there
	    fb[y][x] = c;
        }
    }
}


void gl_draw_pixel(int x, int y, color_t c)
{
    // check if the pixel is in bounds
    if (x >= gl_get_width() || y >= gl_get_height() || x<0 || y<0)
	return;

    // declare the fb as an array of rows
    int (*fb)[fb_get_pitch()/4] = fb_get_draw_buffer();

    // store the given color in there 
    fb[y][x] = c;
}

color_t gl_read_pixel(int x, int y)
{
    // check if the pixel is in bounds
    if (x >= gl_get_width() || y >= gl_get_height() || x<0 || y<0)
        return 0;

    // declare the fb as an array of rows
    int (*fb)[fb_get_pitch()/4] = fb_get_draw_buffer();

    // return the value at the location
    return (color_t)fb[y][x];    
}

void gl_draw_rect(int x, int y, int w, int h, color_t c)
{
    // clipped w and h adjusting for boundaries
    if ((x+w) >= gl_get_width()) 
	w = gl_get_width() - x; 
    if ((y+h) >= gl_get_height())
	h = gl_get_height() - y;
  
    // draw the rectangle
    for (int posy = y; posy < y+h; posy++) {
	for (int posx = x; posx < x+w; posx++) {
	    gl_draw_pixel(posx, posy, c); 
        }
    }
}




void gl_draw_char(int x, int y, char ch, color_t c)
{
    
    unsigned char buf[font_get_glyph_size()];
    if (!font_get_glyph(ch, buf, sizeof(buf)))
	return;

    int startX = x;

    int width = gl_get_char_width();
    int height = gl_get_char_height();

    // clipping width and height adjusting
    // for boundaries
    if ((x+width) >= fb_get_width())
	width = fb_get_width() - x;
    if ((y+height) >= gl_get_height())
	height = gl_get_height() - y;

    unsigned char (*img)[gl_get_char_width()] = (unsigned char (*)[gl_get_char_width()]) buf;

    for (int posy = 0; posy < height; posy++) {
        for (int posx = 0; posx < width; posx++) {
            if (img[posy][posx] == 0xff)
		gl_draw_pixel(x, y, c);
	    x++;
        }
	x = startX;
	y++;
    }

}




void gl_draw_string(int x, int y, const char* str, color_t c)
{
    int i = 0;
    while (str[i] != '\0') {
	gl_draw_char(x, y, str[i], c);
	x += gl_get_char_width();
	i++;
    }
}




unsigned int gl_get_char_height(void)
{
    return font_get_glyph_height();
}




unsigned int gl_get_char_width(void)
{
    return font_get_glyph_width();
}
