#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <stdarg.h>
#include "instruction.h"
#include "utilities.h"
#include "machine.h"
#include "bof.h"

#define MEMORY_SIZE_IN_WORDS 32768
#define NUM_REGISTERS 8

//memory array for the VM
static union mem_u{
    word_type words[MEMORY_SIZE_IN_WORDS];
    uword_type uwords[MEMORY_SIZE_IN_WORDS];
    bin_instr_t instrs[MEMORY_SIZE_IN_WORDS];
}memory;

//initialize the VM
//set initial values for fp, sp, pc
//initialize memory stack
//#Caitlin
void initialize(){}

//open bof
//should call load_instructions and close file when done
//Madigan 9/18
void open_file(const char *filename){

    BOFFILE *f = malloc(sizeof(BOFFILE));
    if (f == NULL) {
    bail_with_error("Memory allocation failed");
    }
    f->filename = filename;
    f->fileptr = fopen(filename, "rb");
    if (f->fileptr == NULL) {
    bail_with_error("Error opening file: %s", filename);
    }

    load_instructions(f);
    //close file and handle errors
    if(fclose(f->fileptr) != 0){
        bail_with_error("Error closing file");
    }

}

//read/decode and put instructions into memory
//will probably use a lot from instructions file
void load_instructions(BOFFILE *f){

    int num_instr = 0;
    while (num_instr >= MEMORY_SIZE_IN_WORDS) {
        bin_instr_t instr = instruction_read(*f);
        memory.instrs[num_instr] = instr;
        num_instr++;
    }

    if(num_instr >= MEMORY_SIZE_IN_WORDS){
        bail_with_error("instr array full");
    }
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
    open_file(filename);
    for(int i=0; i<sizeof(memory.instrs)/sizeof(memory.instrs[0]); i++){
        instr_type t = instruction_type(memory.instrs[i]);
        printf("%d", t);
    }
}

//prints the current state of the memory stack
//could be useful for debugging
void print_state(){}
