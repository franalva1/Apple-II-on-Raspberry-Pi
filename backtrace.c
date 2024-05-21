#include "backtrace.h"
#include "printf.h"

const char *name_of(uintptr_t fn_start_addr)
{

    uintptr_t  * val = (uintptr_t *)(fn_start_addr-4);

    if (*val < 0xff000000){
	
	return "???";

    }

    int lengthOfName = *val & ~0xff000000;

    val -= lengthOfName/4;

    const char * result = (const char *)val;

    return result;
}


int backtrace (frame_t f[], int max_frames)
{

    uintptr_t *cur_fp;
    uintptr_t *saved_lr;
    uintptr_t *saved_fp;

    __asm__("mov %0, fp" : "=r" (cur_fp));

    int n = 0;

    saved_fp = cur_fp - 3;
    saved_lr = cur_fp - 1;

    cur_fp = (uintptr_t *) *saved_fp;

    while (*(saved_fp) != 0 && max_frames > 0) {

	saved_fp = (uintptr_t *)(cur_fp - 3);


	f[n].name = name_of((uintptr_t)*cur_fp-12);
	f[n].resume_addr = *saved_lr;
        f[n].resume_offset = *saved_lr - (uintptr_t)(*cur_fp-12);

	saved_lr = cur_fp - 1;
	cur_fp = (uintptr_t *) *saved_fp;

	n++;

        max_frames--;

    }

    return n;
}


void print_frames (frame_t f[], int n)
{
    for (int i = 0; i < n; i++)
        printf("#%d 0x%x at %s+%d\n", i, f[i].resume_addr, f[i].name, f[i].resume_offset);
}

void print_backtrace (void)
{
    int max = 50;
    frame_t arr[max];

    int n = backtrace(arr, max);
    print_frames(arr, n);   // print frames starting at this function's caller
}
