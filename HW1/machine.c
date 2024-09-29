#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include "instruction.h"
#include "utilities.h"
#include "machine.h"
#include "bof.h"
#include "machine_types.h"
#include "disasm.h"
#include "regname.h"
#include "string.h"//do we need this twice?

#define MEMORY_SIZE_IN_WORDS 32768
#define BYTES_PER_WORD 4

//out file
FILE *out_file;

//memory array for the VM
//the data is shared in a union
static union mem_u{
    word_type words[MEMORY_SIZE_IN_WORDS];
    uword_type uwords[MEMORY_SIZE_IN_WORDS];
    bin_instr_t instrs[MEMORY_SIZE_IN_WORDS];
}memory;

//VM Registers
int GPR[NUM_REGISTERS];
int program_counter, HI, LO;


//booleans
bool tracing = true;
bool halt = false;

//Data Dictionary
int32_t HI = 0;
int32_t LO = 0;

//number of instructions to execute
int num_instr = 0;

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
    
    program_counter = 0; 

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

    printf("trying to open header in init()");
    if (fseek(bf.fileptr, 0, SEEK_SET) != 0) {
        printf("file pointer not at beginning");
    }
    BOFHeader header = bof_read_header(bf);

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

//read/decode and put instructions into memory
//will probably use a lot from instructions file
/*
//this newer load_instructions file did not work, but the old one does work so I put it back in here
void load_instructions(BOFFILE *f){

    //should initialize PC to val specified in bof here

    if(f->fileptr == NULL){
        printf("fileptr null");
    }

    if (fseek(f->fileptr, 0, SEEK_SET) != 0) {
        printf("file pointer not at beginning");
    }

    printf("trying to open header in load_instrs()");
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
        if(ftell(f->fileptr) == EOF){
            printf("\nReached the end of the file");
            printf("%ld %d", ftell(f->fileptr), EOF);
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
*/

void load_instructions(BOFFILE *f){

    if (fseek(f->fileptr, 0, SEEK_SET) != 0) {
        printf("file pointer not at beginning");
    }
    BOFHeader header = bof_read_header(*f);

    int count = header.text_length;
    printf("header text len: %d\n", header.text_length);

    printf("count: %d\n\n", count);

    for(int i=0; i<count; i++){
        memory.instrs[i] = instruction_read(*f);
    }

    int data_count = header.data_length;
    int start = header.data_start_address;
    for(int i=0; i<data_count; i++){
        memory.words[start+i] = bof_read_word(*f);
    }
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
                    printf("\nan NOP_F command was run\n");
                    break;
                case ADD_F:
                    //Benny Comment 9-25 : changed all "formOffset" to "machine_types_formOffset" (easy to mistake function)
                    memory.words[GPR[compi.rt] + machine_types_formOffset(compi.ot)] = memory.words[GPR[1]] + memory.words[GPR[compi.rs] + machine_types_formOffset(compi.os)];
                    break;
                case SUB_F:
                    memory.words[GPR[compi.rt] + machine_types_formOffset(compi.ot)] = memory.words[GPR[1]] - memory.words[GPR[compi.rs] + machine_types_formOffset(compi.os)];
                    break;
                case CPW_F:
                    memory.words[GPR[compi.rt] + machine_types_formOffset(compi.ot)] = memory.words[GPR[compi.rs] + machine_types_formOffset(compi.os)];
                    break;
                case AND_F:
                    memory.uwords[GPR[compi.rt] + machine_types_formOffset(compi.ot)] = memory.uwords[GPR[1]] & memory.uwords[GPR[compi.rs] + machine_types_formOffset(compi.os)];
                    break;
                case BOR_F:
                    memory.uwords[GPR[compi.rt] + machine_types_formOffset(compi.ot)] = memory.uwords[GPR[1]] | memory.uwords[GPR[compi.rs] + machine_types_formOffset(compi.os)];
                    break;
                case NOR_F:
                    memory.uwords[GPR[compi.rt] + machine_types_formOffset(compi.ot)] = ~(memory.uwords[GPR[1]] | memory.uwords[GPR[compi.rs] + machine_types_formOffset(compi.os)]);
                    break;
                case XOR_F:
                    memory.uwords[GPR[compi.rt] + machine_types_formOffset(compi.ot)] = memory.uwords[GPR[1]] ^ memory.uwords[GPR[compi.rs] + machine_types_formOffset(compi.os)];
                    break;
                case LWR_F:
                    GPR[compi.rt] = memory.words[GPR[compi.rs] + machine_types_formOffset(compi.os)];
                    break;
                case SWR_F:
                    memory.words[GPR[compi.rt] + machine_types_formOffset(compi.ot)] = GPR[compi.rs];
                    break;
                case SCA_F:
                    memory.words[GPR[compi.rt] + machine_types_formOffset(compi.ot)] = GPR[compi.rs] + machine_types_formOffset(compi.os);
                    break;
                case LWI_F:
                    memory.words[GPR[compi.rt] + machine_types_formOffset(compi.ot)] = memory.words[memory.words[GPR[compi.rs] +machine_types_formOffset(compi.os)]];
                    break;
                case NEG_F:
                    // Benny Comment 9-25 : unsure of use of ~, shouldnt this be a - or a -1 * memory.uwords[GPR[compi.rs]]?
                    memory.words[GPR[compi.rt] + machine_types_formOffset(compi.ot)] = ~memory.words[GPR[compi.rs] + machine_types_formOffset(compi.os)];
                    break;
                default:
                    bail_with_error("Illegal Comp Instruction");
                    break;
            }
        break;
        }//end of comp_instr_t case
        //Benny
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
                    printf("odd sys_f call in othc that would make no sense so this would never actually print okay if this prints ill be really confused <3");


                default:
                {
                    bail_with_error("Illegal Other Comp Instruction");
                    break;
                }
            }
            break;
        }
        //Madigan
        case syscall_instr_type:
        {
            syscall_instr_t syscalli = bi.syscall;
            //look in enum for syscall_type
            switch(syscalli.code){
                case print_str_sc:
                    {
                        memory.words[GPR[SP]] = printf("%s", &memory.words[GPR[syscalli.reg] + machine_types_formOffset(syscalli.offset)]);
                        break;
                    }
                case print_char_sc:
                    {
                        int char_to_put = memory.words[GPR[syscalli.reg] + machine_types_formOffset(syscalli.offset)];
                        memory.words[GPR[SP]] = fputc(char_to_put, stdout);
                        break;
                    }
                case read_char_sc:
                    {
                        int char_to_read = getc(stdin);
                        memory.words[GPR[syscalli.reg] + machine_types_formOffset(syscalli.offset)] = getc(stdin);
                        break;
                    }
                case start_tracing_sc:
                    {
                        tracing = true;
                        break;
                    }
                case stop_tracing_sc:
                    {
                        tracing = false;
                        break;
                    }
                case exit_sc:
                    {
                        halt = true;
                        break;
                    }
                default:
                {
                    bail_with_error("Illegal Syscall Instruction");
                    break;
                }
            }
            break;
        }

        //Wesley
        case immed_instr_type:
        {
            immed_instr_t immedi = bi.immed;
            //look in enum for opcodes
            switch(immedi.op){
                // case COMP_O:
                // case OTHC_O:
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
                        if(memory.words[GPR[SP]] == (memory.words[GPR[immedi.reg] + machine_types_formOffset(immedi.offset)])) {
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
            break;
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
            break;
        }

        case error_instr_type:
        {
            bail_with_error("Illegal Instruction Type");
            break;
        }

    }
    // char toPrint[MEMORY_SIZE_IN_WORDS] = toString(bi);
}

void print_instructions(){
    for(int i=0; i<MEMORY_SIZE_IN_WORDS; i++){
        bin_instr_t *ptr = &memory.instrs[i];
        uintptr_t add = (uintptr_t)ptr;
        unsigned int int_add = (unsigned int)add;
        if(add == 0 || instruction_type(*ptr) == comp_instr_type && ptr->comp.op == 0){
            continue;
        }
    instruction_print(out_file, int_add, memory.instrs[i]);
    }
}

//puts the functionality of all the previous functions together
//works as the main function of this file
//call init, open, load, execute and trace 
void run(const char *filename){

    out_file = fopen("out_file.txt", "w"); 

    printf("Run has been called\n\n");

    //Opening BOFFILE
    BOFFILE bf = bof_read_open(filename);
    //printf("trying to open header in run()");
    //BOFHeader bf_header = bof_read_header(bf);

    //Initializing
    initialize(bf);

    //Loading Instructions
    load_instructions(&bf);

    //print instructions (for debugging)
    print_instructions();

    //print initial state after loading instruction
    bin_instr_t bin = blankInstr;
    print_memory_state(bin);
    // instruction_print(stdout, 0, memory.instrs[0]);
    
    program_counter = 0;
    while (!halt) {
        bin = memory.instrs[program_counter];
        program_counter++;

        //Tracing / Program State
        if (tracing)
            print_current_instruction(bin);

        //Executing Instruction
        execute(bin);

        //Printing Memory State
        if (tracing)
            print_memory_state(bin);

    }
    //while (int i=0; i < MEMORY_SIZE_IN_WORDS; i++) {
        //execute(memory.instrs[i]);
    //}


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
    // BOFFILE *f = open_file(filename);
    // load_instructions(f);
    // disasmProgram(stdout, *f);
}

//converts the instruction to a string
//used for the print_state function
/*char * toString(bin_instr_t bin){
    char instr [MEMORY_SIZE_IN_WORDS];//this is the finale
    char one [MEMORY_SIZE_IN_WORDS];
    char two [MEMORY_SIZE_IN_WORDS];
    char three [MEMORY_SIZE_IN_WORDS];
    char four [MEMORY_SIZE_IN_WORDS];
    char five [MEMORY_SIZE_IN_WORDS];
    char six [MEMORY_SIZE_IN_WORDS];

    switch(instruction_type(bin)){
        case comp_instr_type:
        {
            comp_instr_t compi = bin.comp;
            snprintf(one, MEMORY_SIZE_IN_WORDS, "%hu", compi.op);
            snprintf(two, MEMORY_SIZE_IN_WORDS, "%hu", compi.rt);
            snprintf(three, MEMORY_SIZE_IN_WORDS, "%hd", compi.ot);
            snprintf(four, MEMORY_SIZE_IN_WORDS, "%hu", compi.rs);
            snprintf(five, MEMORY_SIZE_IN_WORDS, "%hd", compi.os);
            snprintf(six, MEMORY_SIZE_IN_WORDS, "%hu", compi.func);
            strcpy(instr, one);
            strcat(instr, two);
            strcat(instr, three);
            strcat(instr, four);
            strcat(instr, five);
            strcat(instr, six);
            break;
        }//end of comp_instr_type case
        case other_comp_instr_type:
        {
            other_comp_instr_t othci = bin.othc;
            snprintf(one, MEMORY_SIZE_IN_WORDS, "%hu", othci.op);
            snprintf(two, MEMORY_SIZE_IN_WORDS, "%hu", othci.reg);
            snprintf(three, MEMORY_SIZE_IN_WORDS, "%hd", othci.offset);
            snprintf(four, MEMORY_SIZE_IN_WORDS, "%hd", othci.arg);
            snprintf(five, MEMORY_SIZE_IN_WORDS, "%hu", othci.func);
            strcpy(instr, one);
            strcat(instr, two);
            strcat(instr, three);
            strcat(instr, four);
            strcat(instr, five);
            break;
        }//end of other_comp_instr_type case
        case syscall_instr_type:
        {
            syscall_instr_t syscalli = bin.syscall;
            snprintf(one, MEMORY_SIZE_IN_WORDS, "%hu", syscalli.op);
            snprintf(two, MEMORY_SIZE_IN_WORDS, "%hu", syscalli.reg);
            snprintf(three, MEMORY_SIZE_IN_WORDS, "%hd", syscalli.offset);
            snprintf(four, MEMORY_SIZE_IN_WORDS, "%d", syscalli.code);
            snprintf(five, MEMORY_SIZE_IN_WORDS, "%hu", syscalli.func);
            strcpy(instr, one);
            strcat(instr, two);
            strcat(instr, three);
            strcat(instr, four);
            strcat(instr, five);
            break;
        }//end of syscall_instr_type case
        case immed_instr_type:
        {
            immed_instr_t immedi = bin.immed;
            snprintf(one, MEMORY_SIZE_IN_WORDS, "%hu", immedi.op);
            snprintf(two, MEMORY_SIZE_IN_WORDS, "%hu", immedi.reg);
            snprintf(three, MEMORY_SIZE_IN_WORDS, "%hd", immedi.offset);
            snprintf(four, MEMORY_SIZE_IN_WORDS, "%d", immedi.immed);
            strcpy(instr, one);
            strcat(instr, two);
            strcat(instr, three);
            strcat(instr, four);
            break;
        }//end of immed_instr_type case
        case jump_instr_type:
        {
            jump_instr_t jump = bin.jump;
            snprintf(one, MEMORY_SIZE_IN_WORDS, "%hu", jump.op);
            snprintf(two, MEMORY_SIZE_IN_WORDS, "%u", jump.addr);
            strcpy(instr, one);
            strcat(instr, two);
            break;
        }//end of jump_instr_type case
        default:
        {
            bail_with_error("Illegal Instruction Type");
            break;
        }
    }
    return instr;
}*/

//prints the program counter and the current status of the
//of the general purpose registers
void print_trace_header(){
    //trace header, one per instruction
    printf("      PC: %d", program_counter);
    if(HI != 0 || LO != 0) printf("\tHI: %d\tLO: %d", HI, LO);
    printf("\n");

    printf("GPR[$gp]: %d\tGPR[$sp]: %d\tGPR[$fp]: %d\tGPR[$r3]: %d\t",GPR[0], GPR[1], GPR[2], GPR[3]);
    printf("GPR[$r4]: %d\nGPR[$r5]: %d\tGPR[$r6]: %d\tGPR[$ra]: %d\n",GPR[4], GPR[5], GPR[6], GPR[7]);
}//end of print_trace_header

//prints data from the GPR[GPRindex] to GPR[GPRindex+1] exclusive of GPR[GPRindex+1]
    // void print_data(int GPRindex){
    //     //variable to keep track of the offset from the GPRindex
    //     int memoryOffset = 0;

    //     //variable to keep track of characters in a line
    //     //can't exceed 69
    //     int numChars = 0;
    //     while(memory.words[GPR[GPRindex] + memoryOffset] != memory.words[GPR[GPRindex+1]]){
    //         //skips printing if all 0s until the next nonzero-value or the end of the loop
    //         //the first condition prevents inadvertently accessing an instr
    //         //the second and third conditions ensure the first zero is printed and only
    //         //consecutive 0s are skipped
    //         if((memoryOffset > 1) && (memory.words[GPR[GPRindex] + memoryOffset] == 0) && (memory.words[GPR[GPRindex] + memoryOffset - 1] == 0)){
    //             //prints ellipse iff the next value is nonzero or the end of the loop
    //             //will be reached on the next iteration
    //             if((GPR[GPRindex] + memoryOffset + 1 == GPR[GPRindex + 1]) || (memory.words[GPR[GPRindex] + memoryOffset + 1] != 0)){
    //                 printf("        ...     \n");
    //             }
    //             memoryOffset++;
    //             continue;
    //         }//end of zeroes logic
            
    //         //determines if a newline will need to be printed
    //         numChars = numChars + 10 + count_digits(memory.words[GPR[GPRindex] + memoryOffset]);
    //         if(numChars > 69){
    //             printf("\n");
    //             numChars = 0;
    //         }
    //         printf("    %4d: %d\t", (GPR[GPRindex] + memoryOffset), memory.words[GPR[GPRindex] + memoryOffset]);
    //         memoryOffset++;
    //     }
   /*
    
    
    int MAX_STRING_LENGTH = 80;
    int TAB_LENGTH = 8;

    char printString[MAX_STRING_LENGTH];
    char tempString[32];//need to check and make sure this makes sense
    //trying to avoid DMA shenanigans1



    //prints all memory between $gp and $sp
    int memoryIndex = 0;

    while(GPR[0] + memoryIndex < GPR[1]){
        printString[0] = '\0';
        tempString[0] = '\0';
        int tabIndex = 1;//max of 8 tabs

        if(memoryIndex > 1 && (memory.words[GPR[0]+ memoryIndex] == 0) && (memory.words[GPR[0] + memoryIndex -1] == 0)){
            if((GPR[0] + memoryIndex + 1) == GPR[1] || (memory.words[GPR[0]+memoryIndex+1] != 0)){
                printf("        ...     \n");
            }
            memoryIndex++;
            continue;
        }

        for(int index = 0; index < MAX_STRING_LENGTH; index++){
            if(GPR[0]+ memoryIndex == GPR[1]) break;
            strcpy(tempString, (char*)(GPR[0] + memoryIndex));
            int tempStringLength = strlen(tempString);
            int tempIndex = 0;
            //fills in 8 indices of the printString preceding ':'
            for(index = index; index < (tabIndex * TAB_LENGTH); index++){
                //Benny Comment 9-25 : bad practice to use strlen() in a loop header (it calls the function every time the loop runs)
                //Benny Comment 9-25 : will not be an issue nor affect our runtime, just thought id be more nitpicky :D <3
                if(index < TAB_LENGTH - tempStringLength)
                    printString[index] = ' ';
                else {
                    printString[index] = tempString[tempIndex];
                    tempIndex++;
                }
            }
            tempIndex = 0;
            tabIndex++;
            //fills in indices of printString including and succeeding ':'
            printString[index] = ':';
            index++;
            printString[index] = ' ';
            strcpy(tempString, (char*)memory.words[GPR[0] + memoryIndex]);
            tempStringLength = strlen(tempString);
            for(index = index; index < (tabIndex * TAB_LENGTH); index++){
                if(tempIndex < tempStringLength){
                    printString[index] = tempString[tempIndex];
                    tempIndex++;
                }
                else printString[index] = ' ';
            }
            tabIndex++;
            memoryIndex++;
        }
        printf("%s\n", printString);
    }//end of while loop that prints everything between $gp and $sp*/
// }//end of print_data


void print_memory_range(int start, int end) {

    // printf("\nstart: %d, end: %d\n", start, end);

    int tracker = start;
    int num_zeros = 0;
    int num_chars = 0;

    if (start == end) {
        printf("    %4d: %d\t", tracker, memory.words[tracker]);
        return;
    }
        

    while (tracker < end) {

        num_chars = num_chars + 10 + count_digits(memory.words[tracker]);
        if(num_chars > 69){
            printf("\n");
            num_chars = 0;
        }

        if (num_zeros > 0 && memory.words[tracker] == 0) {
            
            while (memory.words[tracker] == 0 && tracker < end)
                tracker++;

            num_zeros == 0;
            printf("        ...     \n");
        }
        
        if (tracker < end) {
            printf("    %4d: %d\t", tracker, memory.words[tracker]);
            num_zeros = 0;
        }

        if (memory.words[tracker] == 0)
            num_zeros++;
        
        tracker++;
    } 

    printf("\n");

}

//determines the number of characters in an int
int count_digits (int number){
    char intAsString[20];
    sprintf(intAsString, "%d", number);
    return strlen(intAsString);
}

void print_current_instruction(bin_instr_t current_instr) {
    if (program_counter != 0) {
        printf("\n==>");
        instruction_print(stdout, program_counter-1, current_instr);
    }
}

//prints the current state of the memory stack
//could be useful for debugging - Benny: Changed for initial state usage
void print_memory_state(bin_instr_t current_instr){

    if (!halt) {
        print_trace_header();
        // print_data(GP);//print between $gp and $sp
        // print_data(SP);//print between $sp and $fp

        print_memory_range(GPR[GP], GPR[SP]);
        print_memory_range(GPR[SP], GPR[FP]+1);
        printf("\n");
        // printf("    %4d: %d\n", GPR[2], memory.words[GPR[2]]);//print $fp
   }
    
}//end of print_state
