#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <stdarg.h>
#include "instruction.h"
#include "utilities.h"
#include "machine.h"
#include "bof.h"
#include "machine_types.h"
#include "disasm.h"

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

    //these are throwing errors
    /*
    registers = {0};
    program_counter = 0; 
    memory = {0};
    */
}

//open bof and return BOFFILE object
//Madigan 9/18
BOFFILE * open_file(const char *filename){

    BOFFILE *f = malloc(sizeof(BOFFILE));
    if (f == NULL) {
    bail_with_error("Memory allocation failed");
    }
    f->filename = filename;
    f->fileptr = fopen(f->filename, "rb");
    if (f->fileptr == NULL) {
    bail_with_error("Error opening file: %s", filename);
    }

    return f;
}

//read/decode and put instructions into memory
//will probably use a lot from instructions file
void load_instructions(BOFFILE *f){

    //should initialize PC to val specified in bof here

    if(f->fileptr == NULL){
        printf("fileptr null");
    }
    if (fseek(f->fileptr, 0, SEEK_SET) != 0) {
        printf("file pointer not at beginning");
    }

    int num_instr = 0;
    fseek(f->fileptr,0,SEEK_END); //move ptr to end
    int end = ftell(f->fileptr); //tell me where the ptr is

    fseek(f->fileptr, 0, SEEK_SET); //move ptr back to beginning
    while (num_instr <= MEMORY_SIZE_IN_WORDS) {
        printf("%d", num_instr);
        if(ftell(f->fileptr) == end){
            printf("Reached the end of the file");
            printf("%ld %d", ftell(f->fileptr), end);
            break;
        }
        bin_instr_t instr = instruction_read(*f);
        //puts the bin_instr_t into memory at memory.instrs
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
    FILE *out_file = fopen("out_file.txt", "w");
    printf("run has been called");
    BOFFILE *f = open_file(filename);
    load_instructions(f);
    for(int i=0; i<sizeof(memory.instrs)/sizeof(memory.instrs[0]); i++){
        //instr_type t = instruction_type(memory.instrs[i]);
        //printf("%d", t);
        bin_instr_t *ptr = &memory.instrs[i];
        uintptr_t add = (uintptr_t)ptr;
        unsigned int int_add = (unsigned int)add;
        fprintf(out_file, "%d    ", i);
        instruction_print(out_file, int_add, memory.instrs[i]);
    }
    fclose(out_file);
}

//when given the "-p" flag, prints out the instructions as written
void print_command (const char *filename){
    BOFFILE *f = open_file(filename);
    //disasmProgram(stdout, filename);
}

//prints the current state of the memory stack
//could be useful for debugging
void print_state(){}
