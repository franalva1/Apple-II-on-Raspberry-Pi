#include "shell.h"
#include "shell_commands.h"
#include "uart.h"
#include "malloc.h"
#include "strings.h"
#include "pi.h"
#include "printf.h"

#define LINE_LEN 80

static input_fn_t shell_read;
static formatted_fn_t shell_printf;

// NOTE TO STUDENTS:
// Your shell commands output various information and respond to user
// error with helpful messages. The specific wording and format of
// these messages would not generally be of great importance, but
// in order to streamline grading, we ask that you aim to match the
// output of the reference version.
//
// The behavior of the shell commands is documented in "shell_commands.h"
// https://cs107e.github.io/header#shell_commands
// The header file gives example output and error messages for all
// commands of the reference shell. Please match this wording and format.
//
// Your graders thank you in advance for taking this care!


static const command_t commands[] = {
    {"help", "help [cmd]",  "print command usage and description", cmd_help},
    {"echo", "echo [args]", "print arguments", cmd_echo},
    {"reboot", "reboot", "reboot the Raspberry Pi", cmd_reboot},
    {"peek", "peek [addr]", "prints the 4-byte value at addr", cmd_peek},
    {"poke", "poke [addr] [val]", "stores val in addr", cmd_poke}
};





int cmd_echo(int argc, const char *argv[])
{
    for (int i = 1; i < argc; ++i)
        shell_printf("%s ", argv[i]);
    shell_printf("\n");
    return 0;
}






int cmd_help(int argc, const char *argv[])
{
 
    // if its just help alone
    if (argc == 1) {

	// go through each command and print the
	// usage and the description
	// i got a weird error when i tried
	// to replace 5 with sizeof(commands)
	// so i left it at 5 and with normal 
	// printf for this reason
	for (int i = 0; i<5; i++) {
		shell_printf("%s  %s \n", commands[i].usage, commands[i].description);
	}	
	return 0;
    }


    // if its help with a command
    for (int i = 0; i<sizeof(commands); i++){

        // if the command name matches with the argument token
        if (strcmp(commands[i].name, argv[1]) == 0) {
            shell_printf("%s  %s\n", commands[i].usage, commands[i].description);
	    return 0;
	}
    }


    shell_printf("error: no such command '%s'\n", argv[1]);

    return -1;
}




int cmd_reboot(int argc, const char *argv[]) {

    uart_putchar(EOT);
    pi_reboot();

    return 0;

}




int cmd_peek(int argc, const char *argv[]) {

    // get the address
    unsigned int * addr = (unsigned int *) strtonum(argv[1], NULL);

    // if they didn't give an address or the address was invalid
    // print out error
    if (argc == 1) {
	
	shell_printf("error: peek expects 1 argument [addr]\n");
	
    } else if (addr == 0 && *argv[1] != '0') {

	shell_printf("error: peek cannot convert '%s'\n", argv[1]);
	
    } else if ((unsigned int)addr%4 != 0) {

	shell_printf("error: peek address must be 4-byte aligned\n");

    } else {

    	shell_printf("%p:  %08x\n", addr, *addr);
	return 0;
    } 

    return -1;

}




int cmd_poke(int argc, const char *argv[]) {

    // get the address
    unsigned int * addr = (unsigned int *) strtonum(argv[1], NULL);

    // get the val 
    unsigned int val = (unsigned int) strtonum(argv[2], NULL);

    // if an argument is missing
    if (argc == 2 || argc == 1) {

	shell_printf("error: poke expects 2 arguments [addr] and [val]\n");
	
    } else if (addr == 0 && *argv[1] != '0') { 

	shell_printf("error: poke cannot convert '%s'\n", argv[1]);

    } else if((val == 0 && *argv[2] != '0')) {

	shell_printf("error: poke cannot convert '%s'\n", argv[2]);
 
    } else if ((unsigned int)addr%4 != 0) {

	shell_printf("error: poke address must be 4-byte aligned");

    } else {

	*addr = val;
        return 0;
    }
 
    return -1;

}



void shell_init(input_fn_t read_fn, formatted_fn_t print_fn)
{
    shell_read = read_fn;
    shell_printf = print_fn;
}

void shell_bell(void)
{
    uart_putchar('\a');
}

void shell_readline(char buf[], size_t bufsize)
{

    char input = shell_read();

    int place = 0;	

    // while we have space to fit the command the user hasn't pressed enter
    while (1) {

	// handling enter
	if (input == '\n') {
	    shell_printf("\n"); 
	    buf[place] = '\n';
     	    buf[place+1] = '\0';
	    return;
	
	// overstepping our boundariese
	} else if (place >= bufsize-1) {
	     buf[place] = '\0';
	     shell_bell();
	     return;

	// handling backspace
	} else if (input == '\b') {
	    
	    // if we are trying to remove 
	    // yet we don't have space
	    if (place==0) {
		shell_bell();
	    } else {
	    // remove it from buffer
	    place--;
	    
	    // undisplay it
	    shell_printf("%c", '\b');
	    shell_printf(" ");
	    shell_printf("%c", '\b');
	    }
 	} else {
	
	    // place the character into buf
	    buf[place] = input;
	    place++;

	    // print out the character
	    shell_printf("%c", input);
	
        }

	// read next character
	input = shell_read();

    }
}

int shell_evaluate(const char *line)
{


    // if command is the enter key or empty string 
    if (line[0] == '\n' || line[0] == '\0') {
 	return 0;
    }


    // length of the line
    int lineLen = strlen(line);

    // our array of tokens
    char * tokens[lineLen];

    // our starting place in tokens
    int place = 0;

    // length of each token
    int count = 0;



    // go through each character in line
    for(int i = 0; i< lineLen; i++) {

	// if we come across the start of a token
	if (line[i] !=  ' ' && line[i] != '\t') {
	

	    // reading for continuous slot of characters
	    while (line[i] !=  ' ' && line[i] != '\t' && line[i] != '\n') {
	
		// increment where we are in the input line
	    	i++;
	
		// increment how long this token is
	    	count++;
	
	    }

	// allocate space for the token
	tokens[place] = malloc(count+1);
	
	// copy the excerpt of the input line 
	// that contains the token into the array
	// tokens[place] 
	memcpy(tokens[place], (const char *)(line+i-count), count);
	
	// ensure the token is a null terminated string
	tokens[place][count] = '\0';

	// for testing purposes
	//  printf("%s\n", tokens[place]);

	// increment our place in tokens
        place++;
        
        // reset the length of the current token to zero
        count=0;	

	}

    }

    // checking for a fully whitespace command
    if (place == 0) {
	return 0;
    }

    // looking up the function pointer for the command 
    for (int i = 0; i<sizeof(commands); i++){
	
	// if the command name matches with the command token
	if (strcmp(commands[i].name, tokens[0]) == 0) { 
	    return commands[i].fn(place, (const char **)tokens);
            
	}
    }


    // if we couldn't run any commands    
    shell_printf("error: no such command '%s'\n", tokens[0]);
    return -1;

}

void shell_run(void)
{
    shell_printf("Welcome to the CS107E shell. Remember to type on your PS/2 keyboard!\n");
    while (1)
    {
        char line[LINE_LEN];

        shell_printf("Pi> ");
        shell_readline(line, sizeof(line));
        shell_evaluate(line);
    }
}
