
#ifndef _MACHINE_H
#define _MACHINE_H
#include "instruction.h"

//maximum height of the stack
#define MAX_ADDR 65536

//run the vm on the given file
extern void run(const char *filename);

//prints the state of the memory stack for debugging
extern void print_state();

void load_instructions(BOFFILE *f);
#endif