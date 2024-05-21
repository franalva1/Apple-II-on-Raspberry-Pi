/*
 * File: malloc.c
 * --------------
 * This is the simple "bump" allocator from lecture.
 * An allocation request is serviced by using sbrk to extend
 * the heap segment.
 * It does not recycle memory (free is a no-op) so when all the
 * space set aside for the heap is consumed, it will not be able
 * to service any further requests.
 *
 * This code is given here just to show the very simplest of
 * approaches to dynamic allocation. You are to replace this code
 * with your own heap allocator implementation.
 */

#include "malloc.h"
#include "printf.h"
#include <stddef.h> // for NULL
#include "strings.h"
#include "backtrace.h"

extern int __bss_end__;

/*
 * The pool of memory available for the heap starts at the upper end of the
 * data section and extend up from there to the lower end of the stack.
 * It uses symbol __bss_end__ from memmap to locate data end
 * and calculates end of stack reservation assuming a 16MB stack.
 *
 * Global variables for the bump allocator:
 *
 * heap_start  location where heap segment starts
 * heap_end    location at end of in-use portion of heap segment
 *
 * count_allocs, count_frees, total_bytes_requested
 *          statistics tracked to assist with debug/validate heap
 */

// Initial heap segment starts at bss_end and is empty
static void *heap_start = &__bss_end__;
static void *heap_end = &__bss_end__;

// Private global variables to track heap statistics
static int count_allocs, count_frees, total_bytes_requested;

// Call sbrk as needed to extend size of heap segment
// Use sbrk implementation as given
void *sbrk(int nbytes)
{
    void *sp;
    __asm__("mov %0, sp" : "=r"(sp));   // get sp register (current stack top)
    char *stack_reserve = (char *)sp - 0x1000000; // allow for 16MB growth in stack

    void *prev_end = heap_end;
    if ((char *)prev_end + nbytes > stack_reserve) {
        return NULL;
    } else {
        heap_end = (char *)prev_end + nbytes;
        return prev_end;
    }
}



// Simple macro to round up x to multiple of n.
// The efficient but tricky bitwise approach it uses
// works only if n is a power of two -- why?
#define roundup(x,n) (((x)+((n)-1))&(~((n)-1)))

void *malloc (size_t nbytes)
{

    nbytes = roundup(nbytes, 8);

    int * place = (int *)heap_start;

    int jump = 0;

    int * result;

    int freeRealEstate = 0;


//
//	searching for an available space
//


    while ((int)place < (int)heap_end) {

	// how many bytes are in this block
        freeRealEstate = *place;

	// how much will we jump to get to the next block
        jump = freeRealEstate/4 + 3;

	// increment to check if the block is free
        place++;

        if (*place == 0) {

	    	
	    // if it's a perfect match
            if (nbytes + 8 == freeRealEstate) {
                
		// say that the segment is in use
		*place = 1;

		// make starting red zone
		place++;
		memset(place, 0x7e, 4);

		// make end red zone
		place++;
		memset(place+nbytes/4, 0x7e, 4);


		// update info and return the segments' start
                count_allocs++;
                return place;


            } else if (nbytes+24 <= freeRealEstate) { // it's a match AND we can fit ATLEAST 1 more segment in there
                
		// say that this current segment is in use
		*place = 1;

		// move down to the segment size portion and adjust it to the size we need
                place--;
                *place = nbytes;

		// make starting red zone
		place += 2;
		memset(place, 0x7e, 4);

		// make ending red zone
		place++; 
		memset(place+nbytes/4, 0x7e, 4);

		// set result to what we need to return
		result = place;
	
		// now we jump to the beginning of the newly made segment
                place += (nbytes/4)+1;

		// denote how many bytes are available in this new segment
                *place = freeRealEstate-nbytes-16;

		// say that the current segment is NOT in use
                place++;
                *place = 0;

		// update info and return the segments' start
                count_allocs++;
                return result;
            }

        }

	// move to the next block
        place += jump;
    
     }


    // increase total bytes by 16 (for header and red zones)
    nbytes += 16;

    // call sbrk to allocate the space for us 
    place = (int *)sbrk(nbytes);

    // check if the request was impossible
    if (place == NULL) return NULL;
    
    // put the nbytes requested in the header 
    *place = (nbytes-16);

    // set the in use bit to 1
    place++;
    *place = (int) 1;

    // make starting red zone
    place++;
    memset(place, 0x7e, 4);

    // make end red zone
    nbytes -= 16;
    place++;
    memset(place+nbytes/4, 0x7e, 4);

    // move spot to beginning of requested segment, update info, return   
    count_allocs++;
    total_bytes_requested+=nbytes;
    return place;
}


void memory_report (void)
{
    printf("\n=============================================\n");
    printf(  "         Mini-Valgrind Memory Report         \n");
    printf(  "=============================================\n");
    printf("FINAL STATS: %d allocs, %d frees, %d total bytes requested", count_allocs, count_frees, total_bytes_requested);
}


void report_damaged_redzone (void *ptr)
{
    printf("\n=============================================\n");
    printf(  " **********  Mini-Valgrind Alert  ********** \n");
    printf(  "=============================================\n");
    printf("Attempt to free address %p that has damaged red zone(s):\n", ptr);
    print_backtrace();
}





void free (void *ptr)
{
    if (ptr == NULL) return;

    // our current position in the heap
    int placeAddress = (int) ptr;
    int * place  = (int *) ptr;

    // checking if a red zone has been pertubed
    place--;
    unsigned char * redZone = (unsigned char *) place;
    for (int i = 0; i < 4; i++) {
	if (*(redZone+i) != 0x7e)
	    report_damaged_redzone(ptr);
    }
    int nbytes = *(place-2);
    place += (nbytes/4) + 1;
    redZone = (unsigned char *) place;
    for (int i = 0; i<4; i++) {
	if (*(redZone+i) != 0x7e)
	    report_damaged_redzone(ptr);
    }
    place = (int *) placeAddress;

    // save our sizeOf segment location
    int * sizeOfHeader = place - 3;

    // set the on bit for this segment to off
    place -= 2;
    *place = 0;

    // move place to the location of the next
    // is in use bit 
    place--;
    int jump = (*place)/4+5;
    place += jump;

    // newRealEstate is how much we are expanding
    // our current segment by
    int newRealEstate = 0;

    // while there is free space and we are in 
    // bounds of the heap
    while (*place == 0 && (int)place < (int)heap_end) {

	// check how many bytes we can add and add them to
	// newRealEstate (how much we are expanding the original
	// segment by 
	place--;
	newRealEstate += *place + 16;

	// move place to the location of the next
	// is in use bit
	jump = (*place)/4+5;
	place+=jump;

    }
    
    *sizeOfHeader += newRealEstate;

    count_frees++;

}





void heap_dump (const char *label)
{
    printf("\n---------- HEAP DUMP (%s) ----------\n", label);
    printf("Heap segment at %p - %p\n", heap_start, heap_end);
    

    int * place = (int *)heap_start;

//    unsigned char *  start = (unsigned char *) heap_start;

    int jump = 0;


    // Use this if you want to see the entire heap
//    while ((int)start < (int)heap_end) {
//    	printf("%p 		%d\n", start, *start);
//   	start++;
//    }


   while ((int)place <  (int)heap_end) {

        printf("SIZE OF HEAP:      %d \n", *place);

        jump = ((int)*place)/4 + 3;

        place++;

        printf("IS IT IN USE:      %d \n", *place);

        place += jump;

    }


    printf("----------  END DUMP (%s) ----------\n", label);
    // heap dump and stats should be consistent
    printf("Stats: %d in-use (%d allocs, %d frees), %d total bytes requested\n\n\n",
     count_allocs - count_frees, count_allocs, count_frees, total_bytes_requested);
}
