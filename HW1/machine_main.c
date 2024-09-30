#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "machine.h"
#include "instruction.h"
#include "disasm.h"

int main(int argc, char *argv[]) {

    const char *command = argv[0];

    //Three args present, check for -p
    if(argc == 3) {
        if (strcmp(argv[1], "-p") == 0) {

            print_command(argv[2]);
            // disasmProgram(out, bof_read_open(argv[2]));
        }
    }

    //Two arg present, should be file location
    else if (argc == 2) {
        printf("%s", argv[1]);
        run(argv[1]);
    }

    //Other than one or two args, run usage
    else {
        void usage(const char *cmdname);

    }



    // //if the second arg is the -p flag, print instructions
    // if (strcmp(argv[0], "-p") == 0) {
	//     //print the instructions
	//     //use disasmProgram(stdout, BOFFILE name)

    //     //commented out for compiling purposes
    //     print_command(argv[2]);
    // }
    // //if the number of arguments is exactly two, run instructions
    // if(argc == 1){
    //     //run() will use the input file 
    //     run(argv[0]);
    // }
    // //if there are more than or less than two arguments, run usage
    // else{
    //     usage(command);
    // }

    return EXIT_SUCCESS;
}

//remind user what to input and exit
void usage(const char *cmdname)
{
    fprintf(stderr, "Usage: %s code-filename\n", cmdname);
    exit(EXIT_FAILURE);
}
