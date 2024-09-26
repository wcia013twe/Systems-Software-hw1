#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <stdarg.h>
#include "instruction.h"
#include "utilities.h"
#include "machine.h"
#include "bof.h"
#include "machine_types.h"
#include "disasm.h"
#include "regname.h"
#include "string.h"

#define MEMORY_SIZE_IN_WORDS 32768

//memory array for the VM
//the data is shared in a union
static union mem_u{
    word_type words[MEMORY_SIZE_IN_WORDS];
    uword_type uwords[MEMORY_SIZE_IN_WORDS];
    bin_instr_t instrs[MEMORY_SIZE_IN_WORDS];
}memory;

//VM Registers
int GPR[NUM_REGISTERS];
int program_counter;

//Data Dictionary
int32_t HI = 0;
int32_t LO = 0;

//Placeholder Blank Instruction (For Comparisons)
bin_instr_t blankInstr;

//initialize the VM
//set initial values for fp, sp, pc
//initialize memory stack
void initialize(BOFFILE bf){
    //global pointer is index 0
    //stack pointer is index 1
    //frame pointer is index 2
    //indices 3-6 are unndesignated
    //return address register is index 7

    //these are throwing errors
    /*
    GPR = {0};
    program_counter = 0; 
    memory = {0};
    */

    //Benny 9/24- I think this should fix these inititializations
    //GPR initializations
    for (int i = 0; i < NUM_REGISTERS; i++)
        GPR[i] = 0;

    //Memory Structure Initialization

        //Blank Initialized bin_instr_t
        blankInstr.comp = (comp_instr_t){0};
        blankInstr.othc = (other_comp_instr_t){0};
        blankInstr.syscall = (syscall_instr_t){0};
        blankInstr.immed = (immed_instr_t){0};
        blankInstr.uimmed = (uimmed_instr_t){0};
        blankInstr.jump = (jump_instr_t){0};

    for (int i = 0; i < MEMORY_SIZE_IN_WORDS; i++) {
        memory.words[i] = 0;
        memory.uwords[i] = 0;
        memory.instrs[i] = blankInstr;
    }

    // const char* something;

    //Open the file
    // BOFFILE bof = bof_read_open(something);

    // BOFHeader header = bof_read_header(bof);

    //The starting address of the code is the initial value of the program counter (PC).
    program_counter = header.text_start_address;

    //Init GP to start of data
    GPR[GP] = header.data_start_address;

    //The starting address of the data section is the address of the first element of the data section and the initial value of the $gp register
    GPR[SP] = header.stack_bottom_addr;
    GPR[FP] = header.stack_bottom_addr;

    // load_instructions(&bof);

    // bof_close(bof);
    // free(bof); not needed for non-pointer
    
}



//open bof and return BOFFILE object
//Madigan 9/18
// BOFFILE * open_file(const char *filename){

//     BOFFILE *f = malloc(sizeof(BOFFILE));
//     if (f == NULL) {
//     bail_with_error("Memory allocation failed");
//     }
//     f->filename = filename;
//     f->fileptr = fopen(f->filename, "rb");
//     if (f->fileptr == NULL) {
//     bail_with_error("Error opening file: %s", filename);
//     }

//     return f;
// }

// BOFFILE *open_file(const char *filename) {
//     BOFFILE *bof = malloc(sizeof(BOFFILE));
//     *bof = bof_read_open(filename);
//     if (bof == NULL) {
//         bail_with_error("Memory allocation failed");
//         exit(EXIT_FAILURE);
//     }

//     return bof;
// }

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

    BOFHeader header = bof_read_header(*f);

    if(!bof_has_correct_magic_number(header)){
        bail_with_error("Invalid magic number");
    }

    int num_instr = 0;
    //instead of moving the pointer to the end and then getting size, we can just use the
    //bof.h function bof_at_eof that returns true when at the end

    //while not at the end and number of instructions did not exceed memory
    while (!bof_at_eof(*f) && num_instr <= MEMORY_SIZE_IN_WORDS) {
        printf("%d", num_instr);
        if(ftell(f->fileptr) == end){
            printf("\nReached the end of the file");
            printf("%ld %d", ftell(f->fileptr), end);
            break;
        }
        bin_instr_t instr = instruction_read(*f);
        //puts the bin_instr_t into memory at memory.instrs
        memory.instrs[num_instr] = instr;
        num_instr++;
    }

    //too many instructions
    if(num_instr >= MEMORY_SIZE_IN_WORDS){
        bail_with_error("instr array full");
    }
    
    printf("Reached the end of the file");
}

//do what the instruction says
//move fp, sp and pc as needed
//will need extra stack management functionsk
void push(word_type value) {
    if (GPR[SP] >= MEMORY_SIZE_IN_WORDS) {
        bail_with_error("Stack overflow");
    }
    memory.words[GPR[SP]++] = value;
}

word_type pop() {
    if (GPR[SP] <= 0) {
        bail_with_error("Stack underflow");
    }
    return memory.words[--GPR[SP]];
}

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
                case ADD_F:
                case SUB_F:
                case CPW_F:
                case AND_F:
                case BOR_F:
                case NOR_F:
                case XOR_F:
                case LWR_F:
                case SWR_F:
                case SCA_F:
                case LWI_F:
                case NEG_F:
                default:
                {
                    bail_with_error("Illegal Comp Instruction");
                    break;
                }
            }
        }
        //Benny 9/24-25
        case other_comp_instr_type:
        {
            other_comp_instr_t othci = bi.othc;
            //look in enum for func1_code
            switch(othci.func){
                case LIT_F:
                    memory.words[GPR[othci.reg] + machine_types_formOffset(othci.offset)] = machine_types_sgnExt(othci.arg);
                    break;

                case ARI_F:
                    GPR[othci.reg] = (GPR[othci.reg] + machine_types_sgnExt(othci.arg));
                    break;

                case SRI_F:
                    GPR[othci.reg] = (GPR[othci.reg] - machine_types_sgnExt(othci.arg));
                    break;

                case MUL_F:
                    int32_t stack_top = memory.words[GPR[SP]];
                    int32_t memory_value = memory.words[GPR[othci.reg] + machine_types_formOffset(othci.offset)];

                    int64_t result = memory_value * stack_top;

                    HI = (result >> 32); //HI 32 Bits
                    LO = (result & 0xFFFFFFFF); //Low 32 Bits

                    break;

                case DIV_F:
                    //Remainder
                    HI = memory.words[GPR[SP]] % (memory.words[GPR[othci.reg] + machine_types_formOffset(othci.offset)]);

                    //Quotient
                    LO = memory.words[GPR[SP]] / (memory.words[GPR[othci.reg] + machine_types_formOffset(othci.offset)]);

                    break;

                case CFHI_F:
                    memory.words[GPR[othci.reg] + machine_types_formOffset(othci.offset)] = HI;
                    break;

                case CFLO_F:
                    memory.words[GPR[othci.reg] + machine_types_formOffset(othci.offset)] = LO;
                    break;

                case SLL_F:
                    memory.uwords[GPR[othci.reg] + machine_types_formOffset(othci.offset)] = memory.uwords[GPR[SP]] << othci.arg;
                    break;

                case SRL_F:
                    memory.uwords[GPR[othci.reg] + machine_types_formOffset(othci.offset)] = memory.uwords[GPR[SP]] >> othci.arg;
                    break;

                case JMP_F:
                    program_counter = memory.uwords[GPR[othci.reg] + machine_types_formOffset(othci.offset)];
                    break;

                case CSI_F:
                    GPR[RA] = program_counter;
                    program_counter = memory.words[GPR[othci.reg] + machine_types_formOffset(othci.offset)];
                    break;

                case JREL_F:
                    program_counter = ((program_counter - 1) + machine_types_formOffset(othci.offset));
                    break;

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
                    // ADD immediate:
                    // memory[GPR[r] + formOffset(o)]
                    // ←(memory[GPR[r] + formOffset(o)]) + sgnExt(i)
                    {
                        word_type address = GPR[immedi.reg] + machine_types_formOffset(immedi.offset);
                        memory.words[address] += machine_types_sgnExt(immedi.immed);
                        break;
                    }
                    
                case ANDI_O:
                    // Bitwise And immediate:
                    // umemory[GPR[r] + formOffset(o)]
                    // ←(umemory[GPR[r] + formOffset(o)]) ∧ zeroExt(i)
                    {
                        uword_type address = GPR[immedi.reg] + machine_types_formOffset(immedi.offset);
                        memory.uwords[address] &= machine_types_zeroExt(immedi.immed);
                        break;
                    }
                case BORI_O:
                    /*
                    Bitwise Or immediate:
                    umemory[GPR[r] + formOffset(o)]
                    ←(umemory[GPR[r] + formOffset(o)]) ∨zeroExt(i)
                    */
                    {
                        uword_type address = GPR[immedi.reg] + machine_types_formOffset(immedi.offset);
                        memory.uwords[address] |= machine_types_zeroExt(immedi.immed);
                        break;
                    }

                case NORI_O:
                    /*
                    Bitwise Not-Or immediate:
                    umemory[GPR[r] + formOffset(o)]
                    ←¬(umemory[GPR[r] + formOffset(o)]) ∨zeroExt(i))
                    */
                    {
                        uword_type address = GPR[immedi.reg] + machine_types_formOffset(immedi.offset);
                        memory.uwords[address] = !(memory.uwords[address] || machine_types_zeroExt(immedi.immed));
                        break;
                    }
                case XORI_O:
                    /*
                    Bitwise Exclusive-Or immediate:
                    umemory[GPR[r] + formOffset(o)]
                    ←(umemory[GPR[r] + formOffset(o)]) xor zeroExt(i)
                    */
                   {
                        uword_type address = GPR[immedi.reg] + machine_types_formOffset(immedi.offset);
                        memory.uwords[address] ^= machine_types_zeroExt(immedi.immed);
                        break;
                   }
                case BEQ_O:
                    /*
                    Branch on Equal:
                    if memory[GPR[$sp]] = memory[GPR[r] + formOffset(o)]
                    then PC ←(PC −1) + formOffset(i)
                    */
                    {
                        if(memory.words[GPR[SP]] == GPR[immedi.reg] + machine_types_formOffset(immedi.offset)){
                            program_counter = (program_counter - 1) + machine_types_formOffset(immedi.immed);
                        };
                        break;
                    }
                case BGEZ_O:
                    /*
                    Branch ≥0:
                    if memory[GPR[r] + formOffset(o)] ≥0
                    then PC ←(PC −1) + formOffset(i
                    */
                    {
                        if(memory.words[GPR[immedi.reg]] + machine_types_formOffset(immedi.offset) >= 0){
                            program_counter = (program_counter - 1) + machine_types_formOffset(immedi.immed);
                        }
                    }
                case BGTZ_O:
                    /*
                    Branch > 0:
                    if memory[GPR[r] + formOffset(o)] > 0
                    then PC ←(PC −1) + formOffset(i)
                    */
                    {
                        if(memory.words[GPR[immedi.reg]] + machine_types_formOffset(immedi.offset) > 0){
                            program_counter = (program_counter - 1) + machine_types_formOffset(immedi.immed);
                        }
                    }
                case BLEZ_O:
                    /*
                    Branch ≤0:
                    if memory[GPR[r] + formOffset(o)] ≤0
                    then PC ←(PC −1) + formOffset(i
                    */
                    {
                        if(memory.words[GPR[immedi.reg]] + machine_types_formOffset(immedi.offset) <= 0){
                            program_counter = (program_counter - 1) + machine_types_formOffset(immedi.immed);
                        }
                    }
                case BLTZ_O:
                    /*
                    ranch < 0:
                    if memory[GPR[r] + formOffset(o)] < 0
                    then PC ←(PC −1) + formOffset(i)
                    */
                    {
                      if(memory.words[GPR[immedi.reg] + machine_types_formOffset(immedi.immed)] < 0) {
                        program_counter = (program_counter - 1) + machine_types_formOffset(immedi.immed);
                      }  
                      break;
                    }
                case BNE_O:
                    /*
                    Branch Not Equal:
                    if memory[GPR[$sp]] ̸= memory[GPR[r] + formOffset(o)]
                    then PC ←(PC −1) + formOffset(i)
                    */
                    {
                        if(memory.words[GPR[SP]] != memory.words[GPR[immedi.reg] + machine_types_formOffset(immedi.offset)]){
                            program_counter = (program_counter - 1) + machine_types_formOffset(immedi.immed);
                        }
                        break;
                    }
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
                    program_counter = machine_types_formAddress(program_counter - 1, jump.addr);
                    break;

                case CALL_O:
                    GPR[RA] = program_counter;
                    program_counter = machine_types_formAddress(program_counter - 1, jump.addr);
                    break;

                case RTN_O:
                    program_counter = GPR[RA];
                    break;

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

//puts the functionality of all the previous functions together
//works as the main function of this file
//call init, open, load, execute and trace 
void run(const char *filename){

    printf("Run has been called\n");

    //Opening BOFFILE
    BOFFILE bf = bof_read_open(filename);
    BOFHeader bf_header = bof_read_header(bf);

    //Initializing
    initialize(bf);

    //Loading Instructions
    load_instructions(&bof);

    //Execute Loop



    // FILE *out_file = fopen("out_file.txt", "w");
    // printf("run has been called");
    // BOFFILE *f = open_file(filename);
    // load_instructions(f);
    // for(int i=0; i<sizeof(memory.instrs)/sizeof(memory.instrs[0]); i++){
    //     //instr_type t = instruction_type(memory.instrs[i]);
    //     //printf("%d", t);
    //     bin_instr_t *ptr = &memory.instrs[i];
    //     uintptr_t add = (uintptr_t)ptr;
    //     unsigned int int_add = (unsigned int)add;
    //     fprintf(out_file, "%d    ", i);
    //     instruction_print(out_file, int_add, memory.instrs[i]);
    // }
    // fclose(out_file);
}

//when given the "-p" flag, prints out the instructions as written
void print_command (const char *filename){
    BOFFILE *f = open_file(filename);
    load_instructions(f);
    disasmProgram(stdout, *f);
}

//prints the current state of the memory stack
//could be useful for debugging
void print_state(){
    for (int i = GPR[SP]; i < MEMORY_SIZE_IN_WORDS; i++) {
        printf("0x%08X: 0x%08X\n", i, memory.words[i]);
    }
    printf("\n");    
}
