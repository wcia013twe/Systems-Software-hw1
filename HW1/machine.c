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

    //Open the file
    // BOFFILE bof = bof_read_open(something);
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
}

void load_instructions(BOFFILE *f){

    if (fseek(f->fileptr, 0, SEEK_SET) != 0) {
        printf("file pointer not at beginning");
    }
    BOFHeader header = bof_read_header(*f);

    int count = header.text_length;

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
    switch(instruction_type(bi)){
        case comp_instr_type:
        {
            comp_instr_t compi = bi.comp;
            switch(compi.func){
                case NOP_F:
                    printf("\nan NOP_F command was run\n");
                    break;
                case ADD_F:
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
                    memory.words[GPR[compi.rt] + machine_types_formOffset(compi.ot)] = ~memory.words[GPR[compi.rs] + machine_types_formOffset(compi.os)];
                    break;
                default:
                    bail_with_error("Illegal Comp Instruction");
                    break;
            }
        break;
        }//end of comp_instr_t case
        case other_comp_instr_type:
        {
            other_comp_instr_t othci = bi.othc;
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
        case syscall_instr_type:
        {
            syscall_instr_t syscalli = bi.syscall;
            switch(syscalli.code){
                case print_str_sc:
                    {
                        memory.words[GPR[SP]] = printf("%s", (char *)&memory.words[GPR[syscalli.reg] + machine_types_formOffset(syscalli.offset)]);
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

        case immed_instr_type:
        {
            immed_instr_t immedi = bi.immed;
            switch(immedi.op){
                case ADDI_O:
                    // ADD immediate:
                    {
                        word_type address = GPR[immedi.reg] + machine_types_formOffset(immedi.offset);
                        memory.words[address] += machine_types_sgnExt(immedi.immed);
                        break;
                    }
                    
                case ANDI_O:
                    // Bitwise And immediate:
                    {
                        uword_type address = GPR[immedi.reg] + machine_types_formOffset(immedi.offset);
                        memory.uwords[address] &= machine_types_zeroExt(immedi.immed);
                        break;
                    }
                case BORI_O:
                     //Bitwise Or immediate:
                    {
                        uword_type address = GPR[immedi.reg] + machine_types_formOffset(immedi.offset);
                        memory.uwords[address] |= machine_types_zeroExt(immedi.immed);
                        break;
                    }

                case NORI_O:
                    //Bitwise Not-Or immediate:
                    {
                        uword_type address = GPR[immedi.reg] + machine_types_formOffset(immedi.offset);
                        memory.uwords[address] = !(memory.uwords[address] || machine_types_zeroExt(immedi.immed));
                        break;
                    }
                case XORI_O:
                    //Bitwise Exclusive-Or immediate:
                   {
                        uword_type address = GPR[immedi.reg] + machine_types_formOffset(immedi.offset);
                        memory.uwords[address] ^= machine_types_zeroExt(immedi.immed);
                        break;
                   }
                case BEQ_O:
                    //Branch on Equal:
                    {
                        if(memory.words[GPR[SP]] == (memory.words[GPR[immedi.reg] + machine_types_formOffset(immedi.offset)])) {
                            program_counter = (program_counter - 1) + machine_types_formOffset(immedi.immed);
                        };
                        break;
                    }
                case BGEZ_O:
                    //Branch ≥0:
                    {
                        if(memory.words[GPR[immedi.reg]] + machine_types_formOffset(immedi.offset) >= 0){
                            program_counter = (program_counter - 1) + machine_types_formOffset(immedi.immed);
                        }
                    }
                case BGTZ_O:
                    //Branch > 0:
                    {
                        if(memory.words[GPR[immedi.reg]] + machine_types_formOffset(immedi.offset) > 0){
                            program_counter = (program_counter - 1) + machine_types_formOffset(immedi.immed);
                        }
                    }
                case BLEZ_O:
                    //Branch ≤0:
                    {
                        if(memory.words[GPR[immedi.reg]] + machine_types_formOffset(immedi.offset) <= 0){
                            program_counter = (program_counter - 1) + machine_types_formOffset(immedi.immed);
                        }
                    }
                case BLTZ_O:
                    //Branch < 0:
                    {
                      if(memory.words[GPR[immedi.reg] + machine_types_formOffset(immedi.immed)] < 0) {
                        program_counter = (program_counter - 1) + machine_types_formOffset(immedi.immed);
                      }  
                      break;
                    }
                case BNE_O:
                    //Branch Not Equal:
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
        case jump_instr_type:
        {
            jump_instr_t jump = bi.jump;
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
}

//WE HAVE TWO OF THESE??? THIS AND print_program
//when given the "-p" flag, prints out the instructions as written
void print_command (const char *filename){
 
    //Opening BOFFILE
    BOFFILE bf = bof_read_open(filename);
    BOFHeader bf_header = bof_read_header(bf);

    //Check its valid bof
    if(!bof_has_correct_magic_number(bf_header)){
        bail_with_error("Error: Incorrect magic number in file: %s\n", filename);
    };

    //prints "Address Instruction"
    instruction_print_table_heading(stdout);

    address_type addr = bf_header.text_start_address;

    /*
        While not at the end of the file
            -get the instruction
            -print the instruction
    
    */
    while (addr < bf_header.text_length + bf_header.text_start_address) {
        bin_instr_t instr = instruction_read(bf);
        instruction_print(stdout, addr, instr);
        addr += 1; //jump to the next instruction
    }

    //print the data section
    int num_zeros = 0;
    int dataLength = bf_header.data_length; 
    address_type address = bf_header.data_start_address;
    while(dataLength > 0){
        //print the data address
        int value = bof_read_word(bf);
        printf("%d: %d\t", address, value);
        dataLength--;
        address++;

        if (value == 0)
            num_zeros++;

        if (num_zeros > 1) {
            while (value == 0 && dataLength > 0) {
                int value = bof_read_word(bf);
                dataLength--;
                address++;
            }

            num_zeros = 0;

            if (value != 0)
                printf("        ...     \n");

        }

    }
    if(dataLength == 0){
        printf("%d: %d\t", address, 0);
    }
    printf("        ...     \n");
    
    bof_close(bf);
    exit(EXIT_SUCCESS);
}

void print_instructions(const char* filename){
    BOFFILE bf = bof_read_open(filename);
    BOFHeader bf_header= bof_read_header(bf);

    address_type addr = bf_header.text_start_address;

    while (addr < bf_header.text_length + bf_header.text_start_address) {
        bin_instr_t instr = instruction_read(bf);
        instruction_print(stdout, addr, instr);
        addr += 1; //jump to the next instruction
    }
}

//puts the functionality of all the previous functions together
//works as the main function of this file
//call init, open, load, execute and trace 
void run(const char *filename){
    //Opening BOFFILE
    BOFFILE bf = bof_read_open(filename);

    //Initializing
    initialize(bf);

    //Loading Instructions
    load_instructions(&bf);

    //print initial state after loading instruction
    bin_instr_t bin = blankInstr;
    print_memory_state(bin);
    
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
}

//WE HAVE TWO OF THESE??? THIS AND print_command
//when given the "-p" flag, prints out the instructions as written
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
}


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


void print_memory_range(int start, int end, int print_checker) {

    int tracker = start;
    int num_zeros = 0;
    int num_chars = 0;
    int print_ellipse = 0;

    if (start == end) {
        printf("    %4d: %d\t", tracker, memory.words[tracker]);
        return;
    }

    while (tracker < end) {
        print_ellipse = 0;

        if (num_zeros > 0 && memory.words[tracker] == 0) {
            
            while (memory.words[tracker] == 0 && tracker < end)
                tracker++;

            num_zeros = 0;
            print_ellipse = 1;
        }

        num_chars = num_chars + 10 + count_digits(memory.words[tracker]) + count_digits(tracker);
        if(num_chars > 70){
            printf("\n");
            num_chars = 0;
        }

        if (print_ellipse == 1 && print_checker == 0) {
            printf("        ...     ");
        }
        
        if (tracker < end) {
            printf("    %4d: %d\t", tracker, memory.words[tracker]);
            num_zeros = 0;
        }

        if (memory.words[tracker] == 0)
            num_zeros++;
        
        tracker++;
    } 

    if (print_checker == 1) {
        num_chars += 16;

        if (num_chars > 70)
            printf("\n");
        printf("        ...     ");

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
        printf("==>");
        instruction_print(stdout, program_counter-1, current_instr);
    }
}

//prints the current state of the memory stack
void print_memory_state(bin_instr_t current_instr){

    if (!halt) {
        print_trace_header();

        print_memory_range(GPR[GP], GPR[SP], 0);
        print_memory_range(GPR[SP], GPR[FP]+1, 0);
        printf("\n");
   }
    
}//end of print_state
