/*
    Simple Stack Machine VM Project for COP3402 - Systems Software w/ Prof. Leavens

    Program Written by Project Group 2: Benedetto Falin, Caitlin Rogers, Madigan Roozen, & Wei-Lin Chou

*/

//Include Statements
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

//Definitions
#define MEMORY_SIZE_IN_WORDS 32768
#define BYTES_PER_WORD 4

//Memory Union Array for VM
static union mem_u{
    word_type words[MEMORY_SIZE_IN_WORDS];
    uword_type uwords[MEMORY_SIZE_IN_WORDS];
    bin_instr_t instrs[MEMORY_SIZE_IN_WORDS];
}memory;

//VM Registers
int GPR[NUM_REGISTERS];
int program_counter, HI, LO;

//Tracing & Halt Placeholders
bool tracing = true;
bool halt = false;

//Data Dictionary
int32_t HI = 0;
int32_t LO = 0;

//Placeholder Blank Instruction (For Comparisons)
bin_instr_t blankInstr;

//Initializes the VM - Initializes Variables for GP, SP, FP, and Memory
void initialize(BOFFILE bf){

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

        //Filling Memory With NOP Instructions
        for (int i = 0; i < MEMORY_SIZE_IN_WORDS; i++) {
            memory.words[i] = 0;
            memory.uwords[i] = 0;
            memory.instrs[i] = blankInstr;
        }

    //Validity Check
    if (fseek(bf.fileptr, 0, SEEK_SET) != 0) {
        printf("file pointer not at beginning");
    }

    //BOFFILE Header
    BOFHeader header = bof_read_header(bf);

    //The starting address of the code is the initial value of the program counter (PC).
    program_counter = header.text_start_address;

    //Init GP to start of data
    GPR[GP] = header.data_start_address;

    //The starting address of the data section is the address of the first element of the data section and the initial value of the $gp register
    GPR[SP] = header.stack_bottom_addr;
    GPR[FP] = header.stack_bottom_addr; 
}

//Loads All Instructions from BOFFILE into Memory.instrs[]
void load_instructions(BOFFILE *f){

    //Base Case for Invalid File
    if (fseek(f->fileptr, 0, SEEK_SET) != 0) {
        printf("file pointer not at beginning");
    }

    //BOFFILE Header
    BOFHeader header = bof_read_header(*f);

    //Reading Instructions from Text Start
    int count = header.text_length;

    for(int i=0; i<count; i++){
        memory.instrs[i] = instruction_read(*f);
    }

    //Reading Data Section of BOFFILE
    int data_count = header.data_length;
    int start = header.data_start_address;

    for(int i=0; i<data_count; i++){
        memory.words[start+i] = bof_read_word(*f);
    }
}

void execute(bin_instr_t bi){
    switch(instruction_type(bi)){

        //Start Computational (comp_instr_t) Instruction Types
        case comp_instr_type:
        {
            //Defining Instruction
            comp_instr_t compi = bi.comp;

            //Ensuring Offset is Valid
            machine_types_check_fits_in_offset(compi.ot);
            machine_types_check_fits_in_offset(compi.os);

            //Switch For Computational Function
            switch(compi.func){
                case NOP_F:
                    printf("\nNothing Should Ever Run here :D\n");
                    break;
                case ADD_F:
     
                    //Add
                    memory.words[GPR[compi.rt] + machine_types_formOffset(compi.ot)] = memory.words[GPR[1]] + memory.words[GPR[compi.rs] + machine_types_formOffset(compi.os)];
                    break;

                case SUB_F:

                    //Subtract
                    memory.words[GPR[compi.rt] + machine_types_formOffset(compi.ot)] = memory.words[GPR[1]] - memory.words[GPR[compi.rs] + machine_types_formOffset(compi.os)];
                    break;

                case CPW_F:

                    //Copy Word
                    memory.words[GPR[compi.rt] + machine_types_formOffset(compi.ot)] = memory.words[GPR[compi.rs] + machine_types_formOffset(compi.os)];
                    break;

                case AND_F:

                    //Bitwise And
                    memory.uwords[GPR[compi.rt] + machine_types_formOffset(compi.ot)] = memory.uwords[GPR[1]] & memory.uwords[GPR[compi.rs] + machine_types_formOffset(compi.os)];
                    break;

                case BOR_F:

                    //Bitwise Or
                    memory.uwords[GPR[compi.rt] + machine_types_formOffset(compi.ot)] = memory.uwords[GPR[1]] | memory.uwords[GPR[compi.rs] + machine_types_formOffset(compi.os)];
                    break;

                case NOR_F:

                    //Bitwise Not-Or
                    memory.uwords[GPR[compi.rt] + machine_types_formOffset(compi.ot)] = ~(memory.uwords[GPR[1]] | memory.uwords[GPR[compi.rs] + machine_types_formOffset(compi.os)]);
                    break;

                case XOR_F:

                    //Bitwise Exclusive-Or
                    memory.uwords[GPR[compi.rt] + machine_types_formOffset(compi.ot)] = memory.uwords[GPR[1]] ^ memory.uwords[GPR[compi.rs] + machine_types_formOffset(compi.os)];
                    break;

                case LWR_F:

                    //Load Word into Register
                    GPR[compi.rt] = memory.words[GPR[compi.rs] + machine_types_formOffset(compi.os)];
                    break;

                case SWR_F:

                    //Store Word from Register
                    memory.words[GPR[compi.rt] + machine_types_formOffset(compi.ot)] = GPR[compi.rs];
                    break;

                case SCA_F:

                    //Store Computed Address
                    memory.words[GPR[compi.rt] + machine_types_formOffset(compi.ot)] = GPR[compi.rs] + machine_types_formOffset(compi.os);
                    break;

                case LWI_F:

                    //Load Word Indirect
                    memory.words[GPR[compi.rt] + machine_types_formOffset(compi.ot)] = memory.words[memory.words[GPR[compi.rs] +machine_types_formOffset(compi.os)]];
                    break;

                case NEG_F:

                    //Negate
                    memory.words[GPR[compi.rt] + machine_types_formOffset(compi.ot)] = (-1) * memory.words[GPR[compi.rs] + machine_types_formOffset(compi.os)];
                    break;

                default:

                    //Comp Instruction Was Not Of Valid Operation
                    bail_with_error("Illegal Comp Instruction");
                    break;

            }

        break;

        } //End of comp_instr_t case

        //Start Other Computational (other_comp_instr_t) Instruction Type
        case other_comp_instr_type:
        {
            //Defining Instruction
            other_comp_instr_t othci = bi.othc;

            //Ensuring Offset is Valid
            machine_types_check_fits_in_offset(othci.offset);

            //Ensuring Arg is Valid
            machine_types_check_fits_in_arg(othci.arg);

            //Switch For Computational Function
            switch(othci.func){
                case LIT_F:

                    //Literal Load
                    memory.words[GPR[othci.reg] + machine_types_formOffset(othci.offset)] = machine_types_sgnExt(othci.arg);
                    break;

                case ARI_F:

                    //Add Register Immediate
                    GPR[othci.reg] = (GPR[othci.reg] + machine_types_sgnExt(othci.arg));
                    break;

                case SRI_F:

                    //Subtract Register Immediate
                    GPR[othci.reg] = (GPR[othci.reg] - machine_types_sgnExt(othci.arg));
                    break;

                case MUL_F:

                    //Multiply
                    int32_t stack_top = memory.words[GPR[SP]];
                    int32_t memory_value = memory.words[GPR[othci.reg] + machine_types_formOffset(othci.offset)];
                    int64_t result = memory_value * stack_top;

                    //HI 32 Bits
                    HI = (result >> 32);

                    //Low 32 Bits
                    LO = (result & 0xFFFFFFFF); 

                    break;

                case DIV_F:

                    //Divide
                        //Remainder
                        HI = memory.words[GPR[SP]] % (memory.words[GPR[othci.reg] + machine_types_formOffset(othci.offset)]);

                        //Quotient
                        LO = memory.words[GPR[SP]] / (memory.words[GPR[othci.reg] + machine_types_formOffset(othci.offset)]);

                        break;

                case CFHI_F:

                    //Copy from HI
                    memory.words[GPR[othci.reg] + machine_types_formOffset(othci.offset)] = HI;
                    break;

                case CFLO_F:

                    //Copy from LO
                    memory.words[GPR[othci.reg] + machine_types_formOffset(othci.offset)] = LO;
                    break;

                case SLL_F:
                    //Ensure Shift is Valid
                    machine_types_check_fits_in_shift(othci.arg);

                    //Shift Left Logical
                    memory.uwords[GPR[othci.reg] + machine_types_formOffset(othci.offset)] = memory.uwords[GPR[SP]] << othci.arg;
                    break;

                case SRL_F:
                    //Ensure Shift is Valid
                    machine_types_check_fits_in_shift(othci.arg);

                    //Shift Right Logical
                    memory.uwords[GPR[othci.reg] + machine_types_formOffset(othci.offset)] = memory.uwords[GPR[SP]] >> othci.arg;
                    break;

                case JMP_F:

                    //Jump
                    program_counter = memory.uwords[GPR[othci.reg] + machine_types_formOffset(othci.offset)];
                    break;

                case CSI_F:

                    //Call Subroutine Indirectly
                    GPR[RA] = program_counter;
                    program_counter = memory.words[GPR[othci.reg] + machine_types_formOffset(othci.offset)];
                    break;

                case JREL_F:

                    //Jump Relative to Address
                    program_counter = ((program_counter - 1) + machine_types_formOffset(othci.offset));
                    break;

                case SYS_F:

                    //System Call Function - Shouldn't Be Found Here: Returned as Proper System Call Instruction Type Before Switch
                    printf("odd sys_f call in othc that would make no sense so this would never actually print okay if this prints ill be really confused <3");
                    break;

                default:
                {
                    //Other Comp Instruction Was Not Of Valid Operation
                    bail_with_error("Illegal Other Comp Instruction");
                    break;
                }
            }

            break;

        } //End of other_comp_instr_t case

        //Start System Call (syscall_instr_t) Instruction Type
        case syscall_instr_type:
        {
            //Defining Instruction
            syscall_instr_t syscalli = bi.syscall;

            //Ensure Offset is Valid
            machine_types_check_fits_in_offset(syscalli.offset);

            //Switch for System Call Code
            switch(syscalli.code){
                case print_str_sc:

                    //Print String
                    memory.words[GPR[SP]] = printf("%s", (char *)&memory.words[GPR[syscalli.reg] + machine_types_formOffset(syscalli.offset)]);
                    break;

                case print_char_sc:
                    
                    //Print Character
                    int char_to_put = memory.words[GPR[syscalli.reg] + machine_types_formOffset(syscalli.offset)];
                    memory.words[GPR[SP]] = fputc(char_to_put, stdout);
                    break;
                    
                case read_char_sc:
                    
                    //Read Character
                    memory.words[GPR[syscalli.reg] + machine_types_formOffset(syscalli.offset)] = getc(stdin);
                    break;
                    
                case start_tracing_sc:
                    
                    //Start Tracing Output
                    tracing = true;
                    break;
                    
                case stop_tracing_sc:
                    
                    //Stop Tracing Output
                    tracing = false;
                    break;
                    
                case exit_sc:
                    
                    //Exit/Stop Program
                    halt = true;
                    break;
                    
                default:
                    
                    //System Call Instruction Was Not Of Valid Instruction
                    bail_with_error("Illegal Syscall Instruction");
                    break;
                
            }

            break;

        } //End System Call (syscall_instr_t) Instruction Type

        //Start Immediate (immed_instr_t) Instruction Type
        case immed_instr_type:
        {
            //Defining Instruction
            immed_instr_t immedi = bi.immed;

            //Ensuring Offset is Valid
            machine_types_check_fits_in_offset(immedi.offset);

            //Switch For Immediate Opcode
            switch(immedi.op){
                case ADDI_O: {
                    
                    //Add Immediate
                    word_type address = GPR[immedi.reg] + machine_types_formOffset(immedi.offset);
                    memory.words[address] += machine_types_sgnExt(immedi.immed);
                    break;
                    
                }
                case ANDI_O: {

                    //Bitwise And Immediate
                    uword_type address = GPR[immedi.reg] + machine_types_formOffset(immedi.offset);
                    memory.uwords[address] &= machine_types_zeroExt(immedi.immed);
                    break;
                   
                }
                case BORI_O: {

                    //Definining Unsigned Immediate Instruction
                    uimmed_instr_t uimmedi = bi.uimmed;

                    //Ensuring Offset is Valid
                    machine_types_check_fits_in_offset(uimmedi.offset);

                    //Bitwise Or Immediate
                    uword_type address = GPR[uimmedi.reg] + machine_types_formOffset(uimmedi.offset);
                    memory.uwords[address] |= machine_types_zeroExt(uimmedi.uimmed);
                    break;
                    
                }
                case NORI_O: {

                    //Definining Unsigned Immediate Instruction
                    uimmed_instr_t uimmedi = bi.uimmed;

                    //Ensuring Offset is Valid
                    machine_types_check_fits_in_offset(uimmedi.offset);

                    //Bitwise Not-Or Immediate
                    uword_type address = GPR[uimmedi.reg] + machine_types_formOffset(uimmedi.offset);
                    memory.uwords[address] = !(memory.uwords[address] || machine_types_zeroExt(uimmedi.uimmed));
                    break;
                    
                }
                case XORI_O: {

                    //Definining Unsigned Immediate Instruction
                    uimmed_instr_t uimmedi = bi.uimmed;

                    //Ensuring Offset is Valid
                    machine_types_check_fits_in_offset(uimmedi.offset);

                    //Bitwise Exclusive-Or Immediate
                    uword_type address = GPR[uimmedi.reg] + machine_types_formOffset(uimmedi.offset);
                    memory.uwords[address] = memory.uwords[address] ^ machine_types_zeroExt(uimmedi.uimmed);
                    break;
                   
                }
                case BEQ_O: {

                    //Ensuring Offset is Valid
                    machine_types_check_fits_in_offset(immedi.immed);

                    //Branch on Equal
                    if(memory.words[GPR[SP]] == (memory.words[GPR[immedi.reg] + machine_types_formOffset(immedi.offset)])) {
                        program_counter = (program_counter - 1) + machine_types_formOffset(immedi.immed);
                    }
                    break;
                   
                } 
                case BGEZ_O: {

                    //Ensuring Offset is Valid
                    machine_types_check_fits_in_offset(immedi.immed);

                    //Branch ≥0
                    if(memory.words[GPR[immedi.reg]] + machine_types_formOffset(immedi.offset) >= 0){
                        program_counter = (program_counter - 1) + machine_types_formOffset(immedi.immed);
                    }
                  
                }  
                case BGTZ_O: {

                    //Ensuring Offset is Valid
                    machine_types_check_fits_in_offset(immedi.immed);

                    //Branch > 0
                    if(memory.words[GPR[immedi.reg]] + machine_types_formOffset(immedi.offset) > 0){
                        program_counter = (program_counter - 1) + machine_types_formOffset(immedi.immed);
                    }
                    
                }
                case BLEZ_O: {

                    //Ensuring Offset is Valid
                    machine_types_check_fits_in_offset(immedi.immed);

                    //Branch ≤0
                    if(memory.words[GPR[immedi.reg]] + machine_types_formOffset(immedi.offset) <= 0){
                        program_counter = (program_counter - 1) + machine_types_formOffset(immedi.immed);
                    }
                   
                } 
                case BLTZ_O: {

                    //Ensuring Offset is Valid
                    machine_types_check_fits_in_offset(immedi.immed);

                    //Branch < 0
                    if(memory.words[GPR[immedi.reg] + machine_types_formOffset(immedi.immed)] < 0) {
                        program_counter = (program_counter - 1) + machine_types_formOffset(immedi.immed);
                    }  
                    break;
                    
                }
                case BNE_O: {

                    //Ensuring Offset is Valid
                    machine_types_check_fits_in_offset(immedi.immed);

                    //Branch Not Equal
                    if(memory.words[GPR[SP]] != memory.words[GPR[immedi.reg] + machine_types_formOffset(immedi.offset)]){
                        program_counter = (program_counter - 1) + machine_types_formOffset(immedi.immed);
                    }
                    break;
                    
                }
                default:

                    //Immediate Type Instruction Was Not Of Valid Operation
                    bail_with_error("Illegal Immediate Instruction");
                    break;
                
            }

            break;

        } //End Immediate (immed_instr_t) Instruction Type

        //Start Jump (jump_instr_t) Instruction type
        case jump_instr_type:
        {
            //Defining Instruction
            jump_instr_t jump = bi.jump;

            //Switch For Jump Instruction Opcode
            switch(jump.op){
                case JMPA_O: {

                    //Defining Address
                    address_type jump_addr = machine_types_formAddress(program_counter - 1, jump.addr);

                    //Ensure Address is Valid
                    machine_types_check_fits_in_addr(jump_addr);

                    //Jump to Given Address
                    program_counter = jump_addr;
                    break;

                }
                case CALL_O: {

                    //Defining Address
                    address_type jump_addr = machine_types_formAddress(program_counter - 1, jump.addr);

                    //Ensure Address is Valid
                    machine_types_check_fits_in_addr(jump_addr);

                    //Call Subroutine
                    GPR[RA] = program_counter;
                    program_counter = jump_addr;
                    break;

                }
                case RTN_O:

                    //Return from Subroutine
                    program_counter = GPR[RA];
                    break;

                default:

                    //Jump Instruction Not Of Valid Opcode
                    bail_with_error("Illegal Jump Instruction");
                    break;

            }

            break;

        } //End Jump (jump_instr_t) Instruction type

        //Error Instruction Type Received
        case error_instr_type:
        {
            bail_with_error("Illegal Instruction Type");
            break;
        }

    }
}

//Runs the Entire SSM Vm Program - Initializes, Loads, Executes, and Traces
void run(const char *filename){
    //Opening BOFFILE
    BOFFILE bf = bof_read_open(filename);

    //Initializing
    initialize(bf);

    //Loading Instructions
    load_instructions(&bf);

    //Placeholder Instruction
    bin_instr_t bin = blankInstr;

    //Initial Memory State Printout
    print_memory_state(bin);
    
    //PC Intializer
    program_counter = 0;

    //Execution Loop - While Not Halted
    while (!halt) {

        //Collect Instruction & Incriment PC
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

    } //End Execution Loop

} //End Run

//Handle -p Argument - Does All Startup Tasks Without Executing the VM
void print_program (const char *filename){

    //Opening BOFFILE
    BOFFILE bf = bof_read_open(filename);
    BOFHeader bf_header = bof_read_header(bf);

    //Initializing
    initialize(bf);

    //Loading Instructions
    load_instructions(&bf);
    
    //Print Header
    instruction_print_table_heading(stdout);

    //Printing Instructions
    print_instructions(filename);

    //Print Data Section
    print_memory_range(GPR[GP], GPR[GP] + bf_header.data_length+1, 1);

} //End Print_Program

//Prints The Memory Addresses and Data From start (inclusive) to end (exlusive) - Somewhat Different Format Implimentation When Running For -p Handling
void print_memory_range(int start, int end, int print_checker) {

    //Initializations
    int tracker = start;
    int num_zeros = 0;
    int num_chars = 0;
    int print_ellipse = 0;

    //Only 1 Location to Print
    if (start == end) {
        printf("    %4d: %d\t", tracker, memory.words[tracker]);
        return;
    }

    //More than 1 Item to Print
    while (tracker < end) {
        print_ellipse = 0;

        //2nd Zero in Sequence Was Found
        if (num_zeros > 0 && memory.words[tracker] == 0) {
            
            //Do Not Print Until Non-zero Found
            while (memory.words[tracker] == 0 && tracker < end) {
                tracker++;
                num_zeros++;
            }

            //Boolean to Print Ellipse
            if (num_zeros > 1)
                print_ellipse = 1;

            //Resetting 0s
            num_zeros = 0;
        }

        //If Ellipse is Set to Print and This is NOT Print_Program
        if (print_ellipse == 1 && print_checker == 0) {

            //Print
            printf("        ...     ");
            num_chars += 16;

            //Check if Newline Needed
            if (num_chars > 59) {
                printf("\n");
                num_chars = 0;
            }

        }
        
        //Printing Memory Locations and Data
        if (tracker < end) {

            //Print Instruction & Data
            printf("    %4d: %d\t", tracker, memory.words[tracker]);

            //Update 0s Since non-zero Entry
            num_zeros = 0;

            //Update num_chars for Newline Checking
            num_chars += 4 + (4 - count_digits(tracker)) + count_digits(tracker) + 2 + count_digits(memory.words[tracker]) + 1;

        }

        //Check if Newline Needed
        if (num_chars > 59) {
            printf("\n");
            num_chars = 0;
        }

        //Increases 0s if 0 Sequence Found
        if (memory.words[tracker] == 0)
            num_zeros++;
        
        //Tracker for Memory Locations Incriment
        tracker++;
    } 

    //Handles print_program - Slightly Different Formatting
    if (print_checker == 1) {
        
        //Check if Newline Needed
        if (num_chars > 59) {
            printf("\n");
        }

        //Print Ellipse
        printf("        ...     ");

        //Update num_chars
        num_chars += 16;

    }

    //Final Newline
    printf("\n");

}

//Returns # of Digits (length) of A Given Number
int count_digits (int number){
    char intAsString[20];
    sprintf(intAsString, "%d", number);
    return strlen(intAsString);
}

//Prints All Memory-Loaded Instructions To stdout
void print_instructions(const char* filename) {

    //BOFFILE & BOFFILE Header
    BOFFILE bf = bof_read_open(filename);
    BOFHeader bf_header = bof_read_header(bf);

    //Initial Intruction Address
    address_type addr = bf_header.text_start_address;

    //Loop to Print Each Instruction
    while (addr < bf_header.text_start_address + bf_header.text_length) {
        bin_instr_t instr = memory.instrs[addr];
        instruction_print(stdout, addr, instr);
        addr++;
    }
}

//Prints Header of Trace With Populated Registers
void print_trace_header(){
    
    //PC Header / HI LO If Needed
    printf("      PC: %d", program_counter);
    if(HI != 0 || LO != 0) printf("\tHI: %d\tLO: %d", HI, LO);
    printf("\n");

    //Registers
    printf("GPR[$gp]: %d\tGPR[$sp]: %d\tGPR[$fp]: %d\tGPR[$r3]: %d\t",GPR[0], GPR[1], GPR[2], GPR[3]);
    printf("GPR[$r4]: %d\nGPR[$r5]: %d\tGPR[$r6]: %d\tGPR[$ra]: %d\n",GPR[4], GPR[5], GPR[6], GPR[7]);

} //End of print_trace_header

//Prints the Current Instruction Passed In To Function
void print_current_instruction(bin_instr_t current_instr) {

    //Will Not Print For Initial Instruction
    if (program_counter != 0) {
        printf("==>");
        instruction_print(stdout, program_counter-1, current_instr);
    }
}

//Prints Current State of Memory Stack - All Registers and Populated Memory Locations for Tracing Output
void print_memory_state(bin_instr_t current_instr){

    //Only While Not Halted
    if (!halt) {

        //Header - Registers
        print_trace_header();

        //Memory Locations & Data
        print_memory_range(GPR[GP], GPR[SP], 0);
        print_memory_range(GPR[SP], GPR[FP]+1, 0);
        printf("\n");
   }
    
} //End of print_memory_state
