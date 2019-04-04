#include "main.h"
#include "memory.h"
#include "decode.h"
#include "devices.h"
#include "execute.h"
#include "ALU.h"
#include "print_contents.h"
#include "file_reader.h"

uint32_t number_tstates_executed = 0;
DecodeControl control;
DecodeControl control_swap;
DecodeControl control_save;

uint8_t swap_reg;
uint8_t reg_a_save;
uint8_t reg_b_save;
uint8_t instruction_reg_save;

int main(int argc, char *argv[]) {
    if(argc != 2) {
        printf("Wrong number of arguments WANT 1 <testprogramfilename.asm>\n");
        exit(1);
    }


//    printf("Printing initial memory contents.\n");
//    print_all_contents();

    read_file(argv[1]);
    printf("Loading test program into memory.\n");
    print_all_contents();
    getchar();

    // Counter to let us track which instruction we're execution
    int instruction_count = 1;

    control = init_decode_control(control);
    control_swap = init_decode_control(control);
    control_save = init_decode_control(control);
    // Loop until program execution halts

    for (;;) {
        //STAGE 1 - control IF/ID
        T1_execute(control.t1_control[control.current_cycle]);
        T2_execute(control.t2_control[control.current_cycle]);
        T3_execute(control.t3_control[control.current_cycle]);
        //save registers
        reg_a_save = mem.reg_a;
        reg_b_save = mem.reg_b;
        instruction_reg_save = mem.instruction_reg;
        //move program counter to next instruction
        mem.address_stack[0] += control.byte_size - 1;

        //save control
        control_swap = control;
        control = control_save;
        control_save = control_swap;

        printf("Finished instruction %d. System state:\n", instruction_count);
        print_all_contents();
        getchar();

        //STAGE 2 - control IF/ID
        T1_execute(control.t1_control[control.current_cycle]);
        T2_execute(control.t2_control[control.current_cycle]);
        T3_execute(control.t3_control[control.current_cycle]);
        //restore registers for control registers and save registers for control
        swap_reg = mem.reg_a;
        mem.reg_a = reg_a_save;
        reg_a_save = swap_reg;

        swap_reg = mem.reg_b;
        mem.reg_b = reg_b_save;
        reg_b_save = swap_reg;

        swap_reg = mem.instruction_reg;
        mem.instruction_reg = instruction_reg_save;
        instruction_reg_save = swap_reg;
        //move program counter back to last instruction

        //save control
        control_swap = control;
        control = control_save;
        control_save = control_swap;

        mem.address_stack[0] -= control.byte_size;

        printf("Finished instruction %d. System state:\n", instruction_count);
        print_all_contents();
        getchar();

        //STAGE 3 - control EX
        T4_execute(control.t4_control[control.current_cycle]);
        T5_execute(control.t5_control[control.current_cycle]);
        control.current_cycle++;
        //execute up to 2 more cycles of control instruction
        for(;control.current_cycle < control.cycle_length; control.current_cycle++) {
            T1_execute(control.t1_control[control.current_cycle]);
            T2_execute(control.t2_control[control.current_cycle]);
            T3_execute(control.t3_control[control.current_cycle]);
            T4_execute(control.t4_control[control.current_cycle]);
            T5_execute(control.t5_control[control.current_cycle]);
        }

        //move program counter back to where IF/ID for control left it
        mem.address_stack[0]++;
        printf("Finished instruction %d. System state:\n", instruction_count);
        print_all_contents();
        instruction_count++;
        getchar();


        printf("Control cycle length = %d\n", control.cycle_length);
        //Does not execute control instruction if any of the three jmp instructions
        if(control.cycle_length == 3 && control.t3_control[1] != DATA_TO_REGB) {
            mem.address_stack[0]--;
            control = init_decode_control(control);
            control_swap = init_decode_control(control);
            control_save = init_decode_control(control);
            continue;
        }

        //restore control registers
        mem.reg_a = reg_a_save;
        mem.reg_b = reg_b_save;
        mem.instruction_reg = instruction_reg_save;

        control = control_save;

        //STAGE 4 - control EX
        T4_execute(control.t4_control[control.current_cycle]);
        T5_execute(control.t5_control[control.current_cycle]);
        control.current_cycle++;
        //execute up to 2 more cycles of control instruction
        for(;control.current_cycle < control.cycle_length; control.current_cycle++) {
            T1_execute(control.t1_control[control.current_cycle]);
            T2_execute(control.t2_control[control.current_cycle]);
            T3_execute(control.t3_control[control.current_cycle]);
            T4_execute(control.t4_control[control.current_cycle]);
            T5_execute(control.t5_control[control.current_cycle]);
        }
        printf("Finished instruction %d. System state:\n", instruction_count);
        print_all_contents();
        instruction_count++;
        getchar();


        //Stages done so repeat
        control = init_decode_control(control);
        control_swap = init_decode_control(control);
        control_save = init_decode_control(control);

    }
	exit(0);
}

