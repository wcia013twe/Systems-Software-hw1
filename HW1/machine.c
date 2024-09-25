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
#include "regname.h"

#define MEMORY_SIZE_IN_WORDS 32768

//memory array for the VM
static union mem_u{
    word_type words[MEMORY_SIZE_IN_WORDS];
    uword_type uwords[MEMORY_SIZE_IN_WORDS];
    bin_instr_t instrs[MEMORY_SIZE_IN_WORDS];
}memory;

//VM Registers
int GPR[NUM_REGISTERS];
int program_counter;

//initialize the VM
//set initial values for fp, sp, pc
//initialize memory stack
void initialize(){
    //global pointer is index 0
    //stack pointer is index 1
    //frame pointer is index 2
    //indices 3-6 are unndesignated
    //return address register is index 7

    //these were throwing errors, please test corrected version
    GPR = {0};
    program_counter = 0; 
    memory.words = {0};
    memory.uwords = {0};
    memory.instrs = {0};
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
void execute(bin_instr_t bi){
    //we'll probably need to open the file and load the instructions
    //but that could also happen in the run
    switch(instruction_type(bi)){
       //Caitlin
        case comp_instr_type:
        {
            comp_instr_t compi = bi.comp;
            //look in enum for func0_code
            switch(compi.func){
                case NOP_F:
                    break;
                case ADD_F:
                    memory.words[GPR[compi.rt] + formOffset(compi.ot)] = memory.words[GPR[1]] + memory.words[GPR[compi.rs] + formOffset(compi.os)];
                    break;
                case SUB_F:
                    memory.words[GPR[compi.rt] + formOffset(compi.ot)] = memory.words[GPR[1]] - memory.words[GPR[compi.rs] + formOffset(compi.os)];
                    break;
                case CPW_F:
                    memory.words[GPR[compi.rt] + formOffset(compi.ot)] = memory.words[GPR[compi.rs] + formOffset(compi.os)];
                    break;
                case AND_F:
                    memory.uwords[GPR[compi.rt] + formOffset(compi.ot)] = memory.uwords[GPR[1]] & memory.uwords[GPR[compi.rs] + formOffset(compi.os)];
                    break;
                case BOR_F:
                    memory.uwords[GPR[compi.rt] + formOffset(compi.ot)] = memory.uwords[GPR[1]] | memory.uwords[GPR[compi.rs] + formOffset(compi.os)];
                    break;
                case NOR_F:
                    memory.uwords[GPR[compi.rt] + formOffset(compi.ot)] = ~(memory.uwords[GPR[1]] | memory.uwords[GPR[compi.rs] + formOffset(compi.os)]);
                    break;
                case XOR_F:
                    memory.uwords[GPR[compi.rt] + formOffset(compi.ot)] = memory.uwords[GPR[1]] ^ memory.uwords[GPR[compi.rs] + formOffset(compi.os)];
                    break;
                case LWR_F:
                    GPR[compi.rt] = memory.words[GPR[compi.rs] + formOffset(compi.os)];
                    break;
                case SWR_F:
                    memory.words[GPR[compi.rt] + formOffset(compi.ot)] = GPR[compi.rs];
                    break;
                case SCA_F:
                    memory.words[GPR[compi.rt] + formOffset(compi.ot)] = GPR[compi.rs] + formOffset(compi.os);
                    break;
                case LWI_F:
                    memory.words[GPR[compi.rt] + formOffset(compi.ot)] = memory.words[memory.words[GPR[compi.rs] +formOffset(compi.os)]];
                    break;
                case NEG_F:
                    memory.words[GPR[compi.rt] + formOffset(compi.ot)] = ~memory.words[GPR[compi.rs] + formOffset(compi.os)];
                    break;
                default:
                    bail_with_error("Illegal Comp Instruction");
                    break;
            }
        }//end of comp_instr_t case
        //Benny
        case other_comp_instr_type:
        {
            other_comp_instr_t othci = bi.othc;
            //look in enum for func1_code
            switch(othci.func){
                case LIT_F:
                case ARI_F:
                case SRI_F:
                case MUL_F:
                case DIV_F:
                case CFHI_F:
                case CFLO_F:
                case SLL_F:
                case SRL_F:
                case JMP_F:
                case CSI_F:
                case JREL_F:
                case SYS_F:
                default:
                {
                    bail_with_error("Illegal Other Comp Instruction");
                    break;
                }
            }
        }
        //Madigan
        case syscall_instr_type:
        {
            syscall_instr_t syscalli = bi.syscall;
            //look in enum for syscall_type
            switch(syscalli.func){
                case print_char_sc:
                case read_char_sc:
                case start_tracing_sc:
                case stop_tracing_sc:
                default:
                {
                    bail_with_error("Illegal Syscall Instruction");
                    break;
                }
            }
        }

        //Wesley
        case immed_instr_type:
        {
            immed_instr_t immedi = bi.immed;
            //look in enum for opcodes
            switch(immedi.op){
                case COMP_O:
                case OTHC_O:
                case ADDI_O:
                case ANDI_O:
                case BORI_O:
                case NORI_O:
                case XORI_O:
                case BEQ_O:
                case BGEZ_O:
                case BGTZ_O:
                case BLEZ_O:
                case BLTZ_O:
                case BNE_O:
                default:
                {
                    bail_with_error("Illegal Immediate Instruction");
                    break;
                }
            }
        }

        //Benny
        case jump_instr_type:
        {
            jump_instr_t jump = bi.jump;
            //look in enum for opcodes
            switch(jump.op){
                case JMPA_O:
                case CALL_O:
                case RTN_O:
                default:
                {
                    bail_with_error("Illegal Jump Instruction");
                    break;
                }
            }
        }

        case error_instr_type:
        {
            bail_with_error("Illegal Instruction Type");
            break;
        }
    }
}

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
    load_instructions(f);
    disasmProgram(stdout, *f);
}

//prints the current state of the memory stack
//could be useful for debugging
void print_state(){}
