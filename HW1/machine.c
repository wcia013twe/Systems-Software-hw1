#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <stdarg.h>
#include "instruction.h"
#include "utilities.h"
#include "machine.h"
#include "machine_types.h"

#define MEMORY_SIZE_IN_WORDS 32768
#define NUM_REGISTERS 8

//memory array for the VM
static union mem_u{
    word_type words[MEMORY_SIZE_IN_WORDS];
    uword_type uwords[MEMORY_SIZE_IN_WORDS];
    bin_instr_t instrs[MEMORY_SIZE_IN_WORDS];
}memory;

//VM Registers
int registers[NUM_REGISTERS];
int program_counter;

//initialize the VM
//set initial values for fp, sp, pc
//initialize memory stack
//#Caitlin
void initialize(){
    //global pointer is index 0
    //stack pointer is index 1
    //frame pointer is index 2
    //indices 3-6 are unndesignated
    //return address register is index 7
    registers = {0};
    program_counter = 0;    
}

//open bof
//should call load_instructions and close file when done
void open_file(){}

//read/decode and put instructions into memory
//will probably use a lot from instructions file
void load_instructions(){
    //perform invariant checks AFTER file is loaded
}

//do what the instruction says
//move fp, sp and pc as needed
//will need extra stack management functions
void execute(){}

//prints stack trace after each instruction
void trace(){}

//puts the functionality of all the previous functions together
//works as the main function of this file
//call init, open, load, execute and trace 
void run(const char *filename){
    printf("run has been called");
}

//prints the current state of the memory stack
//could be useful for debugging
void print_state(){}
