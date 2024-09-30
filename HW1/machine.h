
#ifndef _MACHINE_H
#define _MACHINE_H
#include "instruction.h"

//maximum height of the stack
#define MAX_ADDR 65536

//run the vm on the given file
extern void run(const char *filename);

//prints the instructions provided without executing
void print_program (const char *filename);

void load_instructions(BOFFILE *f);
#endif
