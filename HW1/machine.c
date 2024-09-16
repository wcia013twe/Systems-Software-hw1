#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <stdarg.h>
#include "instruction.h"
#include "utilities.h"
#include "machine.h"

//initialize the VM
//set initial values for fp, sp, pc
//initialize memory stack
void initialize(){}

//open bof
//should call load_instructions and close file when done
void open_file(){}

//read/decode and put instructions into memory
//will probably use a lot from instructions file
void load_instructions(){}

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