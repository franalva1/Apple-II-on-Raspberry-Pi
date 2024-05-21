#include "console.h"
#include "gl.h"
#include "malloc.h"
#include <stdarg.h>
#include "printf.h"
#include "strings.h"

// Please use this amount of space between console rows
const static int LINE_SPACING = 5;
static int line_height;

// position of the cursor
static int curX, curY;

// an array of chars storing the console output
static char * characters;

// limits of 
static int numCols, numRows;

// current position in characters array
static int curPos;

// background color
color_t bg;
color_t fg;

// last X before new line
static int lastX;



void console_init(unsigned int nrows, unsigned int ncols, color_t foreground, color_t background)
{
    // set line height
    line_height = gl_get_char_height() + LINE_SPACING;

    // initialize our number of rows and columns
    numCols = ncols;
    numRows = nrows; 

    // allocate enough sapce for the characters to be outputted
    characters = malloc((nrows*ncols+1)*10);

    // initilialize our GL (with correct sizing)
    gl_init(ncols*gl_get_char_width(),  line_height*nrows, GL_DOUBLEBUFFER);

    // set our background and foreground colors
    bg = background;
    fg = foreground;

    // set our current position in the character array
    curPos = 0;

    // setting our last X
    lastX = (ncols-1)*gl_get_char_width();

    // draw the background
    gl_clear(bg);
	
    // show the background
    gl_swap_buffer();

    // draw the background
    gl_clear(bg);

    // show the background
    gl_swap_buffer();

}

void console_clear(void)
{

    // draw an empty background
    gl_clear(bg);

    // show the empty background
    gl_swap_buffer();


    // draw an empty background
    gl_clear(bg);

    // show the empty background
    gl_swap_buffer();

    // delete all contents in characters
    memset(characters, 0x77, (numRows*numCols+1)*10);     

    characters[0] = '\0';

}


// changed a bit

// will find the first new line in 
// characters (used for scroll to 
// basically find out up to what 
// in characters we are going to 
// cut out
int findFirstNewLine(void) {
    int i = 0;
    while (characters[i] != '\n' && i<numCols)	
	i++;
    return i;
}




static void process_char(char ch)
{

    //printf("We are currently at (%d, %d)\n", curX, curY);


    // if backspace
    if (ch == '\b') {

        // if backspace at beginning of linew
        if (curX == 0) {
	
           // if (curY != 0) {
		curX = lastX;
		curY -= line_height;
		gl_draw_rect(curX, curY, gl_get_char_width(), line_height, bg);
	   // }
        // if normal backspace
        } else { 
	    curX -= gl_get_char_width();
	    gl_draw_rect(curX, curY, gl_get_char_width(), line_height, bg);
	}	
    // if new line
    } else if (ch == '\n') {
       	//lastPos = curX;
        curX = 0;
        curY += line_height;
    // if form feed
    } else if (ch == '\f') {
        gl_clear(bg);
	curX = 0;
        curY = 0;
      
    // if normal char
    } else {

        // draw a char at postition (x, y)
        gl_draw_char(curX, curY, ch, fg);

        // if our x is in bounds, increase it, else, return it to first column
        if ( (curX+gl_get_char_width()) <= lastX) {
            curX += gl_get_char_width();
        } else {
            curY += line_height;
            curX = 0;
        }

    }

}





int console_printf(const char *format, ...)
{

    // declaring our va_list
    va_list ap;
    
    // initializing our va_list
    va_start(ap, format);

    // getting buf ready
    char buf[numRows*numCols];
    size_t bufsize = sizeof(buf);
    memset(buf, 0x77, bufsize);


    // call vsnprintf to write new characters into characters
    curPos += vsnprintf(buf, bufsize, format, ap);
    strlcat(characters, buf, bufsize);
 
    // terminate the end of characters with an empty string
    characters[curPos] = '\0';

    // create x, y, noting where we start drawing on screen
    curX = 0;
    curY = 0;

    int i = 0;

    while( characters[i] != '\0') {

	// if we need to scroll
	if ( curY + 8 > line_height*numRows ) {
	    gl_clear(bg);
	    gl_swap_buffer();
            gl_clear(bg);
            gl_swap_buffer();
	    curY = 0;
	    curX = 0;
            characters = (char *)(1+characters + findFirstNewLine());
	    i = 0;
	}
	process_char(characters[i]);
	i++;
    }

    gl_swap_buffer();
    return curPos;
}

