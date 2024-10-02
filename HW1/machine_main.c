#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "machine.h"
#include "instruction.h"
#include "disasm.h"

//Function Prototype
void usage(int err, int argc, char* argv[]); //Reminds users of error during input / run of program

int main(int argc, char *argv[]) {

    //Three Args Present, Check For -p Arg
    if(argc == 3) {

        //-p Argument Present
        if (strcmp(argv[1], "-p") == 0) {
            //Does All Startup Tasks for VM Except Running/Executing - Prints Instructions and Initial Data
            print_program(argv[2]);
        }

        //-p not present - invalid option
        else {
            usage(1, argc, argv);
        }
    }

    //Two Arg Present, Should Only Be File Location
    else if (argc == 2) {

        //Runs VM as Usual With Input File
        run(argv[1]);
    }

    //Used Arguments Are Incorrect
    else {
        usage(0, argc, argv);
    }

    //Return Exit Success
    return EXIT_SUCCESS;
}

//remind user what to input and exit (assuming first and last arg are file executable and input file respectively)
void usage(int err, int argc, char* argv[])
{
    if (err == 0)
        fprintf(stderr, "Too few/many arguments present. Acceptable usage: %s -p/no-arg %s", argv[0], argv[argc - 1]);
    else if (err == 1)
        fprintf(stderr, "Incorrect argument input. Acceptable usage: %s -p/no-arg %s", argv[0], argv[argc - 1]);

    exit(EXIT_FAILURE);
}
