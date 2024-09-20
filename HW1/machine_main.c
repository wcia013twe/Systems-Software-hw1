#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "machine.h"
#include "instruction.h"
#include "disasm.h"

int main(int argc, char *argv[])
{
    extern void run(const char *filename);
    extern void usage(const char *cmdname);
    const char *command = argv[0];

    //if the second arg is the -p flag, print instructions
    if (strcmp(argv[1], "-p") == 0) {
	    //print the instructions
	    //use disasmProgram(stdout, BOFFILE name)
    }
    //if the number of arguments is eaxctly two, run instructions
    if(argc == 2){
        //run() will use the input file 
        run(argv[1]);
    }
    //if there are more than or less than two arguments, run usage
    else{
        usage(command);
    }

    return EXIT_SUCCESS;
}

//remind user what to input and exit
void usage(const char *cmdname)
{
    fprintf(stderr, "Usage: %s code-filename\n", cmdname);
    exit(EXIT_FAILURE);
}
