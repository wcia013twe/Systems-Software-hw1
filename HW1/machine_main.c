#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "machine.h"
#include "instruction.h"
#include "disasm.h"

int main(int argc, char *argv[]) {

    //Three args present, check for -p
    if(argc == 3) {
        if (strcmp(argv[1], "-p") == 0) {
            print_command(argv[2]);
        }
    }

    //Two arg present, should be file location
    else if (argc == 2) {
        run(argv[1]);
    }

    //Other than one or two args, run usage
    else {
        void usage(const char *cmdname);
    }
    return EXIT_SUCCESS;
}

//remind user what to input and exit
void usage(const char *cmdname)
{
    fprintf(stderr, "Usage: %s code-filename\n", cmdname);
    exit(EXIT_FAILURE);
}
