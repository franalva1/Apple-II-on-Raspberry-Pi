#include "strings.h"

void *memcpy(void *dst, const void *src, size_t n)
{
    /* Copy contents from src to dst one byte at a time */
    char *d = dst;
    const char *s = src;
    while (n--) {
        *d++ = *s++;
    }
    return dst;
}

void *memset(void *dst, int val, size_t n)
{

    // repeatedly put val into dst
    char *temp = dst;
    while (n--) {
	*temp++ = val;   	
    }
    return dst;
}


// ::)

size_t strlen(const char *str)
{
    /* Implementation a gift to you from lab3 */
    size_t n = 0;
    while (str[n] != '\0') {
        n++;
    }
    return n;
}

int strcmp(const char *s1, const char *s2)
{
    // compares character by character according to ascii values
    int n =0;
    if (s1[0] == '\0' && s2[0] == '\0') return 0;	
    while (s1[n] != '\0' && s2[n] != '\0') {
	if (s1[n] > s2[n]) return 1;
	if (s1[n] < s2[n]) return -1;	
    	n++;
    }
    return s1[n] - s2[n];
}

size_t strlcat(char *dst, const char *src, size_t dstsize)
{
    // special case if src is empty string

    if (src[0] == '\0') return strlen(dst);

    // special case if dst is an incorrect string
    
    if (strlen(dst) >= dstsize) return dstsize+strlen(src);

    int dstLen = strlen(dst);

    int srcLen = strlen(src);

    memcpy(dst+strlen(dst), src, dstsize-strlen(dst)-1);

    dst[dstsize-1] = '\0';

    return dstLen+srcLen;

}



//takes in a character and returns the number it represents
unsigned int toNum(const char c) {
    if (c == '0')
	return 0;
    if (c == '1')
	return 1;
    if (c == '2')
	return 2;
    if (c == '3')
	return 3;
    if (c == '4')
	return 4;
    if (c == '5')
	return 5;
    if (c == '6')
	return 6;
    if (c == '7')
	return 7;
    if (c == '8')
	return 8;
    if (c == '9')
	return 9;
    if (c == 'a' || c == 'A')
	return 10;
    if (c == 'b' || c == 'B')
	return 11;
    if (c == 'c' || c == 'C')
	return 12;
    if (c == 'd' || c == 'D')
	return 13;
    if (c == 'e' || c == 'E')
	return 14;
    if (c == 'f' || c == 'F')
	return 15;
    return -1;
}


// checks if the character is a valid decimal digit
unsigned int isDecimalDigit(const char c) {
    if ( c == '1' || c == '2' || c == '3' || c == '4' ||
	c == '5' || c == '6' || c == '7' || c == '8' ||
	c == '9' || c == '0') return 1;
    return 0;
}



// checks if the character is a valid hex digit
unsigned int isHexDigit(const char c) {

    if ( isDecimalDigit(c) == 1 || c == 'a' || c == 'A' || c == 'b' ||
	c == 'B' || c == 'c' || c == 'C' || c == 'd' ||
	c == 'D' || c == 'f' || c == 'F' || c == 'E' ||
	c == 'e') return 1;
    return 0;

}



// finds the length of number passed (as a string)
unsigned int findNumLength(const char *str, int start, unsigned int isHex) {
    int place = start;
    
    if (isHex == 1) {
	while (str[place] != '\0' && isHexDigit(str[place])) 
	    place++;
    } else {
	while (str[place] != '\0' && isDecimalDigit(str[place]))
            place++;
    }
    return place-start;
}



// returns the result of raising base to the power of exp
unsigned int power(int base, int exp) {

    int total = 1;

    for (int i = 0; i<exp; i++) 
	total = total * base;
    return total;

}


unsigned int strtonum(const char *str, const char **endptr)
{

    int total = 0;

    int numLength = 0;

    int place = 0;

    // checks if the number is in hex

    if (str[0] == '0' && str[1] == 'x') {

	// find the length of the number

        numLength = findNumLength(str, 2, 1);

        place = 2;

	// cycle through each digit and compute its value store in total

        while (str[place] != '\0' && numLength > 0){
            total += toNum(str[place]) * power(16, numLength-1);
            place++;
	    numLength--;
        }
    } else {   // if its a decimal number
        numLength = findNumLength(str, 0, 0);
        while (str[place] != '\0' && numLength>0) {
            total += toNum(str[place]) * power(10, numLength-1);
            place++;
	    numLength--;
        }

    }
    
    //updating endptr if need be
    if (endptr != NULL)
    	*endptr = str+place;
    return total;
}
