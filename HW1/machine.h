
#ifndef _MACHINE_H
#define _MACHINE_H
#include "instruction.h"

//maximum height of the stack
#define MAX_ADDR 65536

//run the vm on the given file
extern void run(const char *filename);

//prints the instructions provided without executing
void print_program (const char *filename);

//loads instructions from bof file into memory
void load_instructions(BOFFILE *f);

//prints the data between GP and SP and between SP and FP
void print_memory_state(bin_instr_t current_instr);

//prints the current instruction pointed to by PC
void print_current_instruction(bin_instr_t current_instr);

//counts the number of digits in an int
int count_digits (int number);
#endif
