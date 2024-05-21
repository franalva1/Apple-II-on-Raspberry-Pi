#include "printf.h"
#include <stdarg.h>
#include <stdint.h>
#include "strings.h"
#include "uart.h"

// ::)


/** Prototypes for internal helpers.
 * Typically these would be qualified as static (private to module)
 * but, in order to call them from the test program, we declare them externally
 */
int unsigned_to_base(char *buf, 
                     size_t bufsize, 
                     unsigned int val, 
                     int base, size_t 
                     min_width);
int signed_to_base(char *buf, 
                   size_t bufsize, 
                   int val, 
                   int base, 
                   size_t min_width);


#define MAX_OUTPUT_LEN 1024


// converts a digit into a char

char digitToChar(unsigned int num) {
    if (num == 0) 
	return '0';
    if (num == 1)
	return '1';
    if (num == 2)
	return '2';
    if (num == 3)
	return '3';
    if (num == 4)
	return '4';
    if (num == 5)
	return '5';
    if (num == 6)
	return '6';
    if (num == 7)
	return '7';
    if (num == 8)
	return '8';
    if (num == 9)
	return '9';
    if (num == 10)
	return 'a';
    if (num == 11)
	return 'b';
    if (num == 12)
	return 'c';
    if (num == 13)
	return 'd';
    if (num == 14)
	return 'e';
    if (num == 15)
	return 'f';
    return -1;
}


// finds the number of places a number takes up in its' base

unsigned int numPlaces(unsigned int num, int base) {
    int places = 0;
    if (num == 0)
	return 1;
    while (num>0) {
	num = num/base;
	places++;
    }
    return places;
}





int unsigned_to_base(char *buf, size_t bufsize, unsigned int val, int base, size_t min_width)
{

    int tempSize = numPlaces(val, base);

    // finds out the number of zeros to pad if need be
 
    if (min_width > tempSize) {

	tempSize = min_width;

    }

    char temp[tempSize];


    if (bufsize == 0) {
        if (tempSize >= min_width)
	    return tempSize;
  	return min_width;
    }	
    
    for (int i = tempSize-1; i>=0; i--) {

	temp[i] = digitToChar(val%base);
	val = val/base;

    }

    if (tempSize < bufsize) {
	    for (int i = 0; i<tempSize; i++) {
		buf[i] = temp[i];
	    } buf[tempSize] = '\0';
	    return tempSize;
    } else {
	buf[bufsize-1] = '\0';
	for (int i = 0; i<bufsize-1; i++) 
	    buf[i] = temp[i];

	if (tempSize >= min_width) 
		return tempSize;
	return min_width;
    }

    return 0;
}

int signed_to_base(char *buf, size_t bufsize, int val, int base, size_t min_width)
{
    
    if (val < 0){

	val = val * -1;

	char temp[bufsize-1];

	int result = unsigned_to_base(temp, bufsize-1, val, base, min_width-1);

	for (int i =1; i<bufsize; i++) 
	    buf[i] = temp[i-1];
	buf[0] = '-';
	return result+1;
    }

    return unsigned_to_base(buf, bufsize, val, base, min_width);

}

int vsnprintf(char *buf, size_t bufsize, const char *format, va_list ap)
{

    // throwaway temp string for string building

    char temp[MAX_OUTPUT_LEN];
    memset(temp, '\0', sizeof(temp));


    // our temp buf

    char tbuf[MAX_OUTPUT_LEN];
    size_t tbufsize = sizeof(tbuf);
    memset(tbuf, '\0', tbufsize);


    // our place in buf
    int bPla =0;

    // our place in format
    int fPla =0;


    // width of number (may be changed according to tag) default is 1
    int width =1;


    // goes through each character in format

    while (bPla < (sizeof(tbuf)-1) && format[fPla] != '\0') {

        // if it has a tag do what's necessary

        if (format[fPla] == '%'){
            fPla++;

	    // initializes width to what is required
		
            if (format[fPla] == '0') {
                int counter =0;
                memset(temp, '\0', sizeof(temp));
                while(format[fPla] != 'd' && format[fPla] != 'x'){
                    temp[counter] = format[fPla];
                    counter++;
                    fPla++;
                }
                width = strtonum(temp, NULL); 
            }

            if (format[fPla] == 'c') {
                tbuf[bPla] = (char) va_arg(ap, int);
            }  else if (format[fPla] == 's') {
                tbuf[bPla] = '\0';
                bPla = strlcat(tbuf, (const char*)(va_arg(ap, int)), tbufsize)-1;                                       
            }  else if (format[fPla] == 'd') {
                tbuf[bPla] = '\0';
                bPla += signed_to_base(temp, tbufsize, va_arg(ap, int), 10, width)-1;
                strlcat(tbuf, temp, tbufsize);
            }  else if (format[fPla] == 'x') {  
                tbuf[bPla] = '\0';
                bPla += unsigned_to_base(temp, tbufsize, va_arg(ap, int), 16, width)-1;
                strlcat(tbuf, temp, tbufsize);
            }  else if (format[fPla] == 'p') {
                tbuf[bPla] = '\0';
                bPla += unsigned_to_base(temp, tbufsize, va_arg(ap, int), 16, 8)+1;
                strlcat(tbuf, "0x", tbufsize);
                strlcat(tbuf, temp, tbufsize);
            }  else {
                tbuf[bPla] = '%';
                tbuf[bPla+1] = '\0';
            }
        } else {  // else just copy whatever is in format into tbuf
            tbuf[bPla] =  format[fPla];
        }

	// increment where we are in format and tbuf, also make sure width is 1 again (default)
        fPla++;
        bPla++;
        width =1;
    }

    // make sure tbuf is finished correctly

    tbuf[bPla] = '\0';

    // the return value

    int result = 0;

    int doIt = 0;

    // finds out how many characters we wrote or would've written
    // also copies tbuf into buf

    while (tbuf[result] != '\0') {

        if (result < bufsize-1) {

            buf[result] = tbuf[result];

        } else if (result == bufsize-1) {

            buf[result] = '\0';
 	    doIt = 1;
        }

        result++;

    }

    // terminates buf as necessary

    if (doIt == 0 && tbuf[result] == '\0')
        buf[result] = '\0';

    va_end(ap);

    return result;

}








int snprintf(char *buf, size_t bufsize, const char *format, ...)
{
    
    va_list ap;

    va_start(ap, format);

    if (bufsize == 0)
	return 0;

    int x = vsnprintf(buf, bufsize, format, ap);

    buf[bufsize-1] = '\0';
 
    return x;
}



int printf(const char *format, ...)
{
    
    va_list ap;

    va_start(ap, format);

    char arr[MAX_OUTPUT_LEN];

    vsnprintf(arr, sizeof(arr), format, ap);

    // i'm pretty sure this is what is meant by "hands string over to uart_putstring
    // to do this i needed to include "uart" which I am not sure i was supposed to do 

    return uart_putstring(arr);

}


/* From here to end of file is some sample code and suggested approach
 * for those of you doing the disassemble extension. Otherwise, ignore!
 *
 * The struct insn bitfield is declared using exact same layout as bits are organized in
 * the encoded instruction. Accessing struct.field will extract just the bits
 * apportioned to that field. If you look at the assembly the compiler generates
 * to access a bitfield, you will see it simply masks/shifts for you. Neat!


static const char *cond[16] = {"eq", "ne", "cs", "cc", "mi", "pl", "vs", "vc",
                               "hi", "ls", "ge", "lt", "gt", "le", "", ""};
static const char *opcodes[16] = {"and", "eor", "sub", "rsb", "add", "adc", "sbc", "rsc",
                                  "tst", "teq", "cmp", "cmn", "orr", "mov", "bic", "mvn"};

struct insn  {
    uint32_t reg_op2:4;
    uint32_t one:1;
    uint32_t shift_op: 2;
    uint32_t shift: 5;
    uint32_t reg_dst:4;
    uint32_t reg_op1:4;
    uint32_t s:1;
    uint32_t opcode:4;
    uint32_t imm:1;
    uint32_t kind:2;
    uint32_t cond:4;
};

static void sample_use(unsigned int *addr) {
    struct insn in = *(struct insn *)addr;
    printf("opcode is %s, s is %d, reg_dst is r%d\n", opcodes[in.opcode], in.s, in.reg_dst);
}

*/
