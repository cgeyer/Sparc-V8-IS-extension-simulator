/*
 * SPARC V8 Instruction Set Extension Simulator
 *
 * File: src/gen_sim.c
 * 
 * Copyright (c) 2012 Clemens Bernhard Geyer <clemens.geyer@gmail.com>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy 
 * of this software and associated documentation files (the "Software"), to 
 * deal in the Software without restriction, including without limitation the 
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
 * THE SOFTWARE.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "sparc_target.h"
#include "sparc_v8.h"
#include "sparc.tab.h"
#include "gen_simulator.h"

/*==========================*/ 
/* Internally used pointers */
/*==========================*/ 

/** Pointer to data memory of simulated processor. */
static uint8_t* data_memory = 0;
/** Size of data memory in bytes. */ 
static uint32_t data_memory_size = 0;
/** 
  * Pointer to abstract instruction type of 
  * simulated processor. 
  */
static sparc_instruction* instructions = 0;

/** 
  * Header of the binary file. Contains target-id,
  * memory size and instruction size.
  */
static simulator_header_t header;

/** Pointer to generic simulator object. */
static gen_simulator_t* gen_simulator = 0;

/** Pointer to simulator error function. */
static error_fct_t simerror = 0;

/*=============================*/
/* Sparc register declarations */
/*=============================*/
/** Sparc processor state register */
static uint32_t sparc_psr = PSR_INIT_MASK;
/** Sparc window invalid mask register */
static const uint32_t sparc_wim = WIM_MASK;
/** Sparc Y register for multiply/divide operations */
static uint32_t sparc_y = 0;
/** Sparc program counter */
static uint32_t sparc_pc = 0;
/** Sparc next program counter */
static uint32_t sparc_npc = 1;

/** Sparc global general purpose registers */
static uint32_t sparc_glob_regs[8];
/** Sparc local general purpose registers */
static uint32_t sparc_local_regs[NWINDOWS][8];
/** Sparc input/output global registers */
static uint32_t sparc_inout_regs[NWINDOWS][8];
/** Representing the registers of current window */
static uint32_t* sparc_window_registers[32];

/** Sparc hardware loop state register */
static hwloop_processor_state_t sparc_hwloop_state;

/** Sparc predicate register for predicated blocks/instructions */
static uint32_t	sparc_preg;
/** Sparc processor predication state register */
static predicate_processor_state_t sparc_pred_state;

/** Sparc cycle counter for simulation */
static uint32_t sparc_cycle_counter = 0;
/** local cycle counter which may be printed out */
static uint32_t sparc_cycle_counter_local = 0;

/**
  * @brief Frees all allocated memory for instructions and data memory.
  */
void cleanUp() {

	uint32_t i;

	/* frees all data memory */
	if (data_memory) {
		free(data_memory);
		data_memory = 0;
	}

	/* free instruction operands */
	for (i = 0; i < gen_simulator->getNumberOfInstructions(); i++) {
		if (instructions[i].operands) {
			free(instructions[i].operands);
		}
	}

	/* free instructions */
	if (instructions) {
		free(instructions);
		instructions = 0;
	}
}

/**
  * @brief Reads the file first 10 bytes of the given binary file 
  *        and saves them in the corresponding header fields.
  * @input[in] instream The binary file which will be simulated.
  * @note The binary data of the file is saved in big endian format.
  */
void readFileHeader(FILE* instream) {

	uint16_t target_id = 0;
	uint32_t memory_size = 0;
	uint32_t instruction_size = 0;
	
	int i, value;
	
	/* first two bytes determine target id */
	for (i = 0; i < 2; i++) {
		if ((value = fgetc(instream)) == EOF) {
			cleanUp();
			simerror("Could not read from file!");
		}
		target_id <<= 8;
		target_id |= (uint16_t) ((value) & 0xff);
	}

	header.target_id = target_id;

	/* next four bytes determine memory size in bytes */
	for (i = 0; i < 4; i++) {
		if ((value = fgetc(instream)) == EOF) {
			cleanUp();
			simerror("Could not read from file!");
		}
		memory_size <<= 8;
		memory_size |= (uint32_t) ((value) & 0xff);
	}

	header.memory_size = memory_size;

	/* last four bytes determine size of instruction memory
	   in bytes */
	for (i = 0; i < 4; i++) {
		if ((value = fgetc(instream)) == EOF) {
			cleanUp();
			simerror("Could not read from file!");
		}
		instruction_size <<= 8;
		instruction_size |= (uint32_t) ((value) & 0xff);
	}

	header.instruction_size = instruction_size;
}

/**
  * @brief Reads the contents of the data memory from the 
  *        given binary file. The memory size must be equal 
  *        to the corresponding field of the file header.
  * @param[in] instream The binary file which will be simulated.
  * @note The binary data of the file is saved in big endian format.
  */
void readMemory(FILE* instream) {
	
	uint32_t memory_size = header.memory_size;
	uint32_t i;

	int value;

	data_memory_size = memory_size + FREE_MEMORY_SIZE;
	/* clear last two bits such that memory is always multiple 
	   of 4 bytes */
	data_memory_size &= 0xfffffffc;

	data_memory = malloc(sizeof(uint8_t)*data_memory_size);

	if (!data_memory) {
		cleanUp();
		simerror("Could not allocate data memory!");
	}

	for (i = 0; i < memory_size; i++) {
		if ((value = fgetc(instream)) == EOF) {
			cleanUp();
			simerror("Could not read from file!\n");
		}
		data_memory[i] = (uint8_t) (value & 0xff);
	}


}

/**
  * @brief Resets all internal registers to their initial
  *        states, but does not change (!) the data memory.
  */
void resetSimulator(void) {

	int i;
	int j;

	sparc_psr = PSR_INIT_MASK;
	sparc_y = 0;
	sparc_pc = 0;
	sparc_npc = 1;

	/* initialize global registers */
	for (i = 0; i < 8; i++) {
		sparc_glob_regs[i] = 0;
	}

	/* initialize local and in/out registers */
	for (i = 0; i < NWINDOWS; i++) {
		for (j = 0; j < 8; j++) {
			sparc_inout_regs[i][j] = 0;
			sparc_local_regs[i][j] = 0;
		}
	}
	
	/* assign correct pointers for start window */
	for (i = 0; i < 8; i++) {
		sparc_window_registers[i] = &(sparc_glob_regs[i]);
	}

	for (i = 8; i < 16; i++) {
		sparc_window_registers[i] = &(sparc_inout_regs[NWINDOWS - 1][i - 8]);
	}

	for (i = 16; i < 24; i++) {
		sparc_window_registers[i] = &(sparc_local_regs[NWINDOWS - 1][i - 16]);
	}

	for (i = 24; i < 32; i++) {
		sparc_window_registers[i] = &(sparc_inout_regs[0][i - 24]);
	}

	/* initialize stack pointer */
	*(sparc_window_registers[SP_REGISTER]) = data_memory_size - 4;

	/* initialize return address for main function
	   => check next PC to be equal (END_OF_INS_MEM>>2) */
	*(sparc_window_registers[CALL_ADDR_REGISTER]) = END_OF_INS_MEM - 8;

	/* set correct hardware loop state */
	sparc_hwloop_state.hwloop_state = HWLOOP_STATE_IDLE;
	sparc_hwloop_state.start_address = 0;
	sparc_hwloop_state.end_address = 0;
	sparc_hwloop_state.loop_counter = 0;

	/* set correct predicate processor state */
	sparc_pred_state.predicate_state = PREDICATE_STATE_NONE;
	sparc_pred_state.predicate_condition.icc = 0;

	/* all predicate registers are initially cleared */
	sparc_preg = 0;

	/* clear cycle counters */
	sparc_cycle_counter = 0;
	sparc_cycle_counter_local = 0;

}


/**
  * @brief Changes the current window pointer of the sparc psr and
  *        rearranges all registers.
  * @param[in] inc Determines whether the window pointer shall be
  *                incremented (inc != 0) or decremented (inc = 0).
  * @note This mechanism is described in the Sparc V8 manual, 
  *       chapter 4, p. 23-30.
  */
static void changeCWP(int inc) {
	uint32_t cwp = PSR_GET_CWP(sparc_psr);
	uint32_t new_cwp; 
	uint32_t new_in_cwp; 
	uint32_t i;

	if (inc) {
		new_cwp = (cwp + 1)%NWINDOWS;
	} else {
		new_cwp = (cwp - 1)%NWINDOWS;
	}
	new_in_cwp = (new_cwp + 1)%NWINDOWS;

	/* rearrange out registers */
	for (i = 8; i < 16; i++) {
		sparc_window_registers[i] = &(sparc_inout_regs[new_cwp][i - 8]);
	}

	/* rearrange local registers */
	for (i = 16; i < 24; i++) {
		sparc_window_registers[i] = &(sparc_local_regs[new_cwp][i - 16]);
	}

	/* rearrange in registers */
	for (i = 24; i < 32; i++) {
		sparc_window_registers[i] = &(sparc_inout_regs[new_in_cwp][i - 24]);
	}

	PSR_SET_CWP(sparc_psr, new_cwp);
}

/**
  * @brief Evaluates whether the given bit in the predicate
  * register is set, depending on register number and true/false
  * value. 
  * @param[in] preg Predicate register number to check.
  * @param[in] tf Whether to check true or false bit of the
  * current predicate register. May only be 0 or 1!
  * @return 1 if the current predicate register is set,
  * 0 otherwise.
  */
static int evaluatePred(int preg, int tf) {
	return ((sparc_preg >> (preg*2 + tf)) & 0x1);
}

/**
  * @brief Evaluates whether the icc bits of the psr are set
  *        such that the given icc is fulfilled. 
  * @param[in] icc The integer condition code to check. 
  * @return 1 if the current condition is fulfilled,
  * 0 otherwise.
  */
static int evaluateICC(int icc) {
	/* handle all condition codes as described in the sparc v8 manual, p. 178 */
	int icc_matched = 0;

	switch(icc) {
		case CC_A:
			icc_matched = 1;
			break;
		case CC_N:
			break;
		case CC_NE:
			if (!PSR_GET_Z(sparc_psr)) {
				icc_matched = 1;
			}
			break;
		case CC_E:
			if (PSR_GET_Z(sparc_psr)) {
				icc_matched = 1;
			}
			break;
		case CC_G:
			if (! (PSR_GET_Z(sparc_psr) | 
				  (PSR_GET_N(sparc_psr) ^ PSR_GET_V(sparc_psr))) ) {
				icc_matched = 1;
			}
			break;
		case CC_LE:
			if ( PSR_GET_Z(sparc_psr) | 
				(PSR_GET_N(sparc_psr) ^ PSR_GET_V(sparc_psr)) ) {
				icc_matched = 1;
			}
			break;
		case CC_GE:
			if (! (PSR_GET_N(sparc_psr) ^ PSR_GET_V(sparc_psr)) ) {
				icc_matched = 1;
			}
			break;
		case CC_L:
			if (PSR_GET_N(sparc_psr) ^ PSR_GET_V(sparc_psr)) {
				icc_matched = 1;
			}
			break;
		case CC_GU:
			if ((!PSR_GET_C(sparc_psr)) & (!PSR_GET_Z(sparc_psr))) {
				icc_matched = 1;
			}
			break;
		case CC_LEU:
			if (PSR_GET_C(sparc_psr) | PSR_GET_Z(sparc_psr)) {
				icc_matched = 1;
			}
			break;
		case CC_CC:
			if (!PSR_GET_C(sparc_psr)) {
				icc_matched = 1;
			}
			break;
		case CC_CS:
			if (PSR_GET_C(sparc_psr)) {
				icc_matched = 1;
			}
			break;
		case CC_POS:
			if (!PSR_GET_N(sparc_psr)) {
				icc_matched = 1;
			}
			break;
		case CC_NEG:
			if (PSR_GET_N(sparc_psr)) {
				icc_matched = 1;
			}
			break;
		case CC_VC:
			if (!PSR_GET_V(sparc_psr)) {
				icc_matched = 1;
			}
			break;
		case CC_VS:
			if (PSR_GET_V(sparc_psr)) {
				icc_matched = 1;
			}
			break;
		default:
			gen_simulator->cleanUp();
			simerror("Encountered unkown icc!");
			break;
	}
	return icc_matched;
}

/**
  * @brief Simulates one step and returns 0 if a return from the main
  *        function has been detected.
  * @param[in] outstream The output file stream where to write additional
  *                      information. Currently only used for debugging.
  * @return 1 if there are unhandled instructions, 0 if there is a return
  *         from the main function.
  */
int simulateStep(FILE* outstream) {

	/* get current instruction */
	sparc_instruction* cur_instruction = &(instructions[sparc_pc]);

	/* get current opcode */
	uint32_t opcode = cur_instruction->opcode;
	/* save the number of unhandled operands of the current instruction */
	uint32_t unhandled_operands = cur_instruction->num_operands;
	/* save the address of the first unhandled operand of the current instruction */
	uint32_t operand_iter = 0;
	/* save operands array of the current instruction */
	sparc_operand* operands = cur_instruction->operands;

	/* for all conditional instructions, we need the icc of the instruction */
	uint32_t icc;
	/* destination register number */
	uint32_t dst_reg;
	/* source 1 register number */ 
	uint32_t src1_reg;
	/* source 2 register number */
	uint32_t src2_reg;
	/* source 1 value */
	uint32_t src1_op;
	/* source 2 value */
	uint32_t src2_op;

	/* address of the destination register of current instruction */
	uint32_t* dst_address = 0;
	/* result of the current instruction */
	uint32_t dst_value = 0;

	/* new value for preg for predset/predclear instructions */
	uint32_t next_preg = sparc_preg;

	/* temporary memory address for load/store instructions */
	uint32_t memory_address = 0;
	/* loop variable for load/store instructions */
	int32_t i;

	/* for multiplication and division */
	uint32_t tmp_y_value = 0;
	uint64_t tmp_udivmul_result = 0;
	int64_t tmp_sdivmul_result = 0;

	/* values for instructions influencing the integer condition codes 
	   of the psr */
	uint32_t changes_icc = 0;
	uint32_t next_icc = 0;

	/* save current program counter for call instruction */
	uint32_t cur_pc = sparc_pc;

	/* boolean which saves if the current instruction will be executed */
	uint32_t executed = 0;

	/* calculate next program counter */
	sparc_pc = sparc_npc;
	/* increment npc per default */
	sparc_npc++;

/* 	fprintf(outstream, "PC: %d, nPC: %d\n", cur_pc, sparc_pc); */
/* 	fprintf(outstream, "Opcode: %d\n", opcode); */

	/* if we are in a hardware loop, we have to check
	   whether we have to branch */
	if (sparc_hwloop_state.hwloop_state == HWLOOP_STATE_ACTIVE) {
		/* the next calculated address is the end of the current loop */
		if (sparc_npc == sparc_hwloop_state.end_address) {
			/* decrement loop counter */
			(sparc_hwloop_state.loop_counter)--; 
			if (sparc_hwloop_state.loop_counter > 0) {
				/* if the loop counter is greater than zero, we can branch */
				sparc_npc = sparc_hwloop_state.start_address;
			} else {
				/* otherwise, we exit loop and leave the active state */
				sparc_hwloop_state.hwloop_state = HWLOOP_STATE_IDLE;
			}
		}
	}
		
	switch(opcode) {
		/* reset local cycle counter and print out number of simulated cycles so far */
		case CYCLE_PRINT:
			fprintf(outstream, "Current simulated cycles: %d.\n", sparc_cycle_counter_local);
			/* we do not need a break because cycle counter will be reset anyway... */
		/* reset local cycle counter */
		case CYCLE_CLEAR:
			sparc_cycle_counter_local = 0;
			break;
		case CALL:
			operand_iter = 1;
			unhandled_operands -= 1;
			/* save next program counter value */
			sparc_npc = operands[0].value.labeladdress;	
			/* save current program counter value (byte address!) in o7 */
			*(sparc_window_registers[CALL_ADDR_REGISTER]) = (cur_pc << 2);
			sparc_cycle_counter += CYCLES_INTEGER_INSTR;
			sparc_cycle_counter_local += CYCLES_INTEGER_INSTR;
			break;
		case SETHI:
			dst_reg = operands[0].value.reg;
			if (dst_reg != (G_REGISTER + 0)) {
				dst_address = sparc_window_registers[dst_reg];
			}
			dst_value = (operands[1].value.imm22 << 10);
			operand_iter = 2;
			unhandled_operands -= 2;
			sparc_cycle_counter += CYCLES_INTEGER_INSTR;
			sparc_cycle_counter_local += CYCLES_INTEGER_INSTR;
			break;
		case NOP:
			/* do nothing */
			sparc_cycle_counter += CYCLES_INTEGER_INSTR;
			sparc_cycle_counter_local += CYCLES_INTEGER_INSTR;
			break;
		case BRANCH:
			icc = operands[1].value.icc;
			/* evaluate whether condition codes are matched */
			if (evaluateICC(icc)) {	
				sparc_npc = operands[0].value.labeladdress;
			}
			operand_iter = 2;
			unhandled_operands -= 2;
			sparc_cycle_counter += CYCLES_INTEGER_INSTR;			
			sparc_cycle_counter_local += CYCLES_INTEGER_INSTR;
			break;
		/* all load instructions need the same address calculation */
		case LDSB:
		case LDSH:
		case LDUB:
		case LDUH:
		case LD:
		case LDD:
		case LDSBA:
		case LDSHA:
		case LDUBA:
		case LDUHA:
			/* save destination register */
			dst_reg = operands[0].value.reg;
			if (dst_reg != (G_REGISTER + 0)) {
				dst_address = sparc_window_registers[dst_reg];
			}
			/* calculate memory address */
			src1_reg = operands[1].value.reg;
			memory_address = *(sparc_window_registers[src1_reg]);
			if (operands[2].type == OPERAND_TYPE_REGISTER) {
				src2_reg = operands[2].value.reg;
				src2_op = *(sparc_window_registers[src2_reg]);
			} else {
				src2_op = operands[2].value.simm13;
			}
			memory_address = memory_address + src2_op;
			/* always load 4 bytes */
			dst_value = 0;
			for (i = 0; i < 4; i++) {
				dst_value <<= 8;
				dst_value |= (uint32_t) (data_memory[(memory_address & 0xfffffffc) + i]);
			}
			operand_iter = 3;
			unhandled_operands -= 3;
			/* set cycle counter corresponding to load operation */
			sparc_cycle_counter += CYCLES_LOAD_SINGLE;
			sparc_cycle_counter_local += CYCLES_LOAD_SINGLE;
			break;
		case LDA:
		case LDDA:
			sparc_cycle_counter += CYCLES_LOAD_DOUBLE;
			sparc_cycle_counter_local += CYCLES_LOAD_DOUBLE;
			break;
		/* all store instructions have the same address calculation */
		case STB:
		case STH:
		case ST:
		case STBA:
		case STHA:
		case STA:
			/* save value which will be saved to destination */
			dst_reg = operands[0].value.reg;
			dst_value = *(sparc_window_registers[dst_reg]);
			/* calculate memory address */
			src1_reg = operands[1].value.reg;
			memory_address = *(sparc_window_registers[src1_reg]);
			if (operands[2].type == OPERAND_TYPE_REGISTER) {
				src2_reg = operands[2].value.reg;
				src2_op = *(sparc_window_registers[src2_reg]);
			} else {
				src2_op = operands[2].value.simm13;
			}
			memory_address = memory_address + src2_op;
			operand_iter = 3;
			unhandled_operands -= 3;
			/* set cycle counter corresponding to store operation */
			sparc_cycle_counter += CYCLES_STORE_SINGLE;
			sparc_cycle_counter_local += CYCLES_STORE_SINGLE;
			break;
		case STDA:
		case STD:
			sparc_cycle_counter += CYCLES_STORE_DOUBLE;
			sparc_cycle_counter_local += CYCLES_STORE_DOUBLE;
			break;
		case LDSTUB:
			break;
		case LDSTUBA:
			break;
		case SWAP:
			break;
		case SWAPA:
			break;
		case SAVE:
		case RESTORE:
			/* first get source operands from old window */
			src1_reg = operands[1].value.reg;
			src1_op = *(sparc_window_registers[src1_reg]);
			if (operands[2].type == OPERAND_TYPE_REGISTER) {
				src2_reg = operands[2].value.reg;
				src2_op = *(sparc_window_registers[src2_reg]);
			} else {
				src2_op = operands[2].value.simm13;
			}
			dst_value = src1_op + src2_op;
			/* change current window */
			if (opcode == SAVE) {
				changeCWP(0);
			} else {
				changeCWP(1);
			}
			/* get destination address for new window */
			dst_reg = operands[0].value.reg;
			if (dst_reg != (G_REGISTER + 0)) {
				dst_address = sparc_window_registers[dst_reg];
			}
			operand_iter = 3;
			unhandled_operands -= 3;
			sparc_cycle_counter += CYCLES_INTEGER_INSTR;
			sparc_cycle_counter_local += CYCLES_INTEGER_INSTR;
			break;
		case JUMPL:
			dst_reg = operands[0].value.reg;
			if (dst_reg != (G_REGISTER + 0)) {
				dst_address = sparc_window_registers[dst_reg];
			}
			src1_reg = operands[1].value.reg;
			memory_address = *(sparc_window_registers[src1_reg]);
			if (operands[2].type == OPERAND_TYPE_REGISTER) {
				src2_reg = operands[2].value.reg;
				src2_op = *(sparc_window_registers[src2_reg]);
			} else {
				src2_op = operands[2].value.simm13;
			}
			memory_address = memory_address + src2_op;
			/* address of next instruction is word address */
			memory_address >>= 2;
			operand_iter = 3;
			unhandled_operands -= 3;
			sparc_cycle_counter += CYCLES_INTEGER_INSTR;
			sparc_cycle_counter_local += CYCLES_INTEGER_INSTR;
			break;
		case RD:
			dst_reg = operands[0].value.reg;
			if (dst_reg != (G_REGISTER) + 0) {
				dst_address = sparc_window_registers[dst_reg];
			}
			src1_reg = operands[1].value.reg;
			if (src1_reg != Y_REGISTER_NO) {
				gen_simulator->cleanUp();
				simerror("Unknown destination register for rd instruction!");
			}
			dst_value = sparc_y;
			operand_iter = 2;
			unhandled_operands -= 2;
			sparc_cycle_counter += CYCLES_INTEGER_INSTR;
			sparc_cycle_counter_local += CYCLES_INTEGER_INSTR;
			break;
		case WR:
			dst_reg = operands[0].value.reg;
			if (dst_reg != Y_REGISTER_NO) {
				gen_simulator->cleanUp();
				simerror("Unknown destination register for wr instruction!");
			}
			dst_address = &sparc_y;
			src1_reg = operands[1].value.reg;
			src1_op = *(sparc_window_registers[src1_reg]); 
			if (operands[2].type == OPERAND_TYPE_REGISTER) {
				src2_reg = operands[2].value.reg;
				src2_op = *(sparc_window_registers[src2_reg]);
			} else {
				src2_op = operands[2].value.simm13;
			}
			dst_value = src1_op ^ src2_op;
			operand_iter = 3;
			unhandled_operands -= 3;
			sparc_cycle_counter += CYCLES_INTEGER_INSTR;
			sparc_cycle_counter_local += CYCLES_INTEGER_INSTR;
			break;
		case MOV:
			dst_reg = operands[0].value.reg;
			if (dst_reg != G_REGISTER + 0) {
				dst_address = sparc_window_registers[dst_reg];
			}
			src1_reg = operands[1].value.reg;
			src1_op = *(sparc_window_registers[src1_reg]);
			/* src2 = dst! */
			src2_reg = dst_reg;
			src2_op = *(sparc_window_registers[src2_reg]);
			icc = operands[2].value.icc;
			/* if condition is true, take src1 value, src2 otherwise */
			if (evaluateICC(icc)) {
				dst_value = src1_op;
			} else {
				dst_value = src2_op;
			}
			operand_iter = 3;
			unhandled_operands -= 3;
			sparc_cycle_counter += CYCLES_INTEGER_INSTR;
			sparc_cycle_counter_local += CYCLES_INTEGER_INSTR;
			break;
		case SEL:
			dst_reg = operands[0].value.reg;
			if (dst_reg != G_REGISTER + 0) {
				dst_address = sparc_window_registers[dst_reg];
			}

			/* get source operand 1 */
			if (operands[1].type == OPERAND_TYPE_REGISTER) {
				src1_reg = operands[1].value.reg;
				src1_op = *(sparc_window_registers[src1_reg]);
			} else {
				src1_op = operands[1].value.simm8;
			}

			/* get source operand 2 */
			if (operands[2].type == OPERAND_TYPE_REGISTER) {
				src2_reg = operands[2].value.reg;
				src2_op = *(sparc_window_registers[src2_reg]);
			} else if (operands[2].type == OPERAND_TYPE_SIMM11) {
				src2_op = operands[2].value.simm11;
			} else if (operands[2].type == OPERAND_TYPE_SIMM8) {
				src2_op = operands[2].value.simm8;
			}

			icc = operands[3].value.icc;
			
			/* selection process */
			if (evaluateICC(icc)) {
				dst_value = src1_op;
			} else {
				dst_value = src2_op;
			}

			operand_iter = 4;
			unhandled_operands -= 4;
			sparc_cycle_counter += CYCLES_INTEGER_INSTR;
			sparc_cycle_counter_local += CYCLES_INTEGER_INSTR;

			break;
		case HWLOOP_INIT:
			dst_reg = operands[0].value.loopreg;
			switch (dst_reg) {
				case LOOPS_REGISTER:
					sparc_hwloop_state.start_address = operands[1].value.labeladdress;
					break;
				case LOOPE_REGISTER:
					sparc_hwloop_state.end_address = operands[1].value.labeladdress;
					break;
				case LOOPB_REGISTER:
					/* save the loop bounds into loop counter register */
					if (operands[1].type == OPERAND_TYPE_REGISTER) {
						src1_reg = operands[1].value.reg;
						sparc_hwloop_state.loop_counter = *(sparc_window_registers[src1_reg]); 
					} else {
						sparc_hwloop_state.loop_counter = operands[1].value.imm22;
					}
					break;
				default:
					gen_simulator->cleanUp();
					simerror("Unknown register for hwloop init!");
					break;
			}
			operand_iter = 2;
			unhandled_operands -= 2;
			sparc_cycle_counter += CYCLES_INTEGER_INSTR;
			sparc_cycle_counter_local += CYCLES_INTEGER_INSTR;
			break;
		case HWLOOP_START:
			/* only set current loop state to active */
			sparc_hwloop_state.hwloop_state = HWLOOP_STATE_ACTIVE;
			sparc_cycle_counter += CYCLES_INTEGER_INSTR;
			sparc_cycle_counter_local += CYCLES_INTEGER_INSTR;
			break;
		case PREDBEGIN:
			/* we have predicated blocks on condition code */
			if (operands[0].type == OPERAND_TYPE_ICC) {
				icc = operands[0].value.icc;
				sparc_pred_state.predicate_state = PREDICATE_STATE_ICC;
				sparc_pred_state.predicate_condition.icc = icc;
			} else if (operands[0].type == OPERAND_TYPE_PREG) {
				/* we have predicated blocks on predicate registers */
				src1_reg = operands[0].value.preg;
				sparc_pred_state.predicate_state = PREDICATE_STATE_PREG;
				sparc_pred_state.predicate_condition.preg_condition.preg = src1_reg;
				src2_op = operands[1].value.tf;
				sparc_pred_state.predicate_condition.preg_condition.tf = src2_op;
			} else {
				cleanUp();
				simerror("Unknown operand type for predbegin instruction!");
			}
			sparc_cycle_counter += CYCLES_INTEGER_INSTR;
			sparc_cycle_counter_local += CYCLES_INTEGER_INSTR;
			break;
		case PREDEND:
			/* save that predicated block is finished */
			sparc_pred_state.predicate_state = PREDICATE_STATE_NONE;
			sparc_cycle_counter += CYCLES_INTEGER_INSTR;
			sparc_cycle_counter_local += CYCLES_INTEGER_INSTR;
			break;
		case PREDSET:
			dst_reg = operands[0].value.preg;
			/* if second operand is icc, we have to set t and f on condition */
			if (unhandled_operands > 1 && operands[1].type == OPERAND_TYPE_ICC) {
				icc = operands[1].value.icc;
				if (evaluateICC(icc)) {
					/* current icc is true => set t and clear f preg */
					next_preg &= ~(1<<(2*dst_reg));
					next_preg |= (1<<(2*dst_reg + 1));
				} else {
					/* current icc is false => clear t and set f preg */
					next_preg &= ~(1<<(2*dst_reg + 1));
					next_preg |= (1<<(2*dst_reg));
				}
				operand_iter = 2;
				unhandled_operands -= 2;
			} else {
				/* predset does not depend on icc => set t and f bit  */
				next_preg |= (1<<(2*dst_reg));
				next_preg |= (1<<(2*dst_reg + 1));
				operand_iter = 1;
				unhandled_operands -= 1;
			}
			sparc_cycle_counter += CYCLES_INTEGER_INSTR;
			sparc_cycle_counter_local += CYCLES_INTEGER_INSTR;
			break;
		case PREDCLEAR:
			/* clear t and f predicate register */
			dst_reg = operands[0].value.preg;
			next_preg &= ~(1<<(2*dst_reg));
			next_preg &= ~(1<<(2*dst_reg + 1));
			operand_iter = 1;
			unhandled_operands -= 1;
			sparc_cycle_counter += CYCLES_INTEGER_INSTR;
			sparc_cycle_counter_local += CYCLES_INTEGER_INSTR;
			break;
		case UNKNOWN: 
			fprintf(stderr, "UNKOWN opcode = %d\n", opcode);
			cleanUp();
			simerror("Not supported opcode encoutered!");
			break;
		/* equal for all arithmetic/logic instructions */
		default:
			/* save address of destination register */
			dst_reg = operands[0].value.reg;
			if (dst_reg != (G_REGISTER + 0)) {
				dst_address = sparc_window_registers[dst_reg];
			}
			/* save value of src1 register */
			src1_reg = operands[1].value.reg;
			src1_op = *(sparc_window_registers[src1_reg]);
			/* depending on type of src2, save either contents of
			   register or the sign extended immediate value */
			if (operands[2].type == OPERAND_TYPE_REGISTER) {
				src2_reg = operands[2].value.reg;
				src2_op = *(sparc_window_registers[src2_reg]);
			} else {
				src2_op = operands[2].value.simm13;
			}
			/* save that we have handled the first three operands */
			operand_iter = 3;
			unhandled_operands -= 3;
			/* all integer instructions take the same amount of cycles */
			sparc_cycle_counter += CYCLES_INTEGER_INSTR;
			sparc_cycle_counter_local += CYCLES_INTEGER_INSTR;
			break;
	}

	/* handle all arithmetic/logic instructions */
	switch (opcode) {
		case AND:
		case ANDCC:
			dst_value = src1_op & src2_op;
			break;
		case ANDN:
		case ANDNCC:
			dst_value = ~(src1_op & src2_op);
			break;
		case OR:
		case ORCC:
			dst_value = src1_op | src2_op;
			break;
		case ORN:
		case ORNCC:
			dst_value = ~(src1_op | src2_op);
			break;
		case XOR:
		case XORCC:
			dst_value = src1_op ^ src2_op;
			break;
		case XNOR:
		case XNORCC:
			dst_value = ~(src1_op ^ src2_op);
			break;
		case SLL:
			dst_value = src1_op << src2_op;
			break;
		case SRL:
			dst_value = src1_op >> src2_op;
			break;
		case SRA:
			dst_value = src1_op;
			for (i = 0; i < (int32_t) src2_op; i++) {
				dst_value >>= 1;
				/* sign extension */
				if (src1_op & 0x80000000) {
					dst_value |= 0x80000000;
				}
			}
			break;
		case ADD:
		case ADDCC:
		/* as we currently do not handle any traps,
		   tagged add are the same as addcc */
		case TADDCC:
		case TADDCCTV:
			dst_value = src1_op + src2_op;
			break;
		case ADDX:
		case ADDXCC:
			dst_value = src1_op + src2_op + PSR_GET_C(sparc_psr);
			break;
		case SUB:
		case SUBCC:
		case TSUBCC:
		case TSUBCCTV:
		/* as we currently do not handle any traps,
		   tagged sub are the same as subcc */
			dst_value = src1_op - src2_op;
/* 			fprintf(outstream, "SUBCC: %d - %d = %d\n", src1_op, src2_op, dst_value); */
			break;
		case SUBX:
		case SUBXCC:
			dst_value = src1_op - src2_op - PSR_GET_C(sparc_psr);
			break;
		case MULSCC:
			break;
		case UMUL:
		case UMULCC:
			tmp_udivmul_result = ((uint64_t) src1_op) * ((uint64_t) src2_op);
			dst_value = (uint32_t) (tmp_udivmul_result & 0xffffffffL);
			tmp_y_value = (uint32_t) ((tmp_udivmul_result >> 32) & 0xffffffffL);
			sparc_cycle_counter += (CYCLES_MUL - CYCLES_INTEGER_INSTR);
			sparc_cycle_counter_local += (CYCLES_MUL - CYCLES_INTEGER_INSTR);
			break;
		case SMUL:
		case SMULCC:
			tmp_sdivmul_result = ((int64_t) src1_op) * ((int64_t) ((int32_t) src2_op));
			dst_value = (uint32_t) (tmp_sdivmul_result & 0xffffffffL);
			tmp_y_value = (uint32_t) ((tmp_sdivmul_result >> 32) & 0xffffffffL);
			sparc_cycle_counter += (CYCLES_MUL - CYCLES_INTEGER_INSTR);
			sparc_cycle_counter_local += (CYCLES_MUL - CYCLES_INTEGER_INSTR);
			break;
		case UDIV:
		case UDIVCC:
			if (src2_op == 0) {
				gen_simulator->cleanUp();
				simerror("Encountered division by zero!");
			}
			tmp_udivmul_result = (uint64_t) sparc_y;
			tmp_udivmul_result <<= 32;
			tmp_udivmul_result |= src1_op;
			tmp_udivmul_result = tmp_udivmul_result / (uint64_t) src2_op;
			dst_value = (uint32_t) (tmp_udivmul_result & 0xffffffffL);
			sparc_cycle_counter += (CYCLES_DIV - CYCLES_INTEGER_INSTR);
			sparc_cycle_counter_local += (CYCLES_DIV - CYCLES_INTEGER_INSTR);
			break;
		case SDIV:
		case SDIVCC:
			if (src2_op == 0) {
				gen_simulator->cleanUp();
				simerror("Encountered division by zero!");
			}
			tmp_sdivmul_result = (int64_t) ((int32_t) sparc_y);
			tmp_sdivmul_result <<= 32;
			tmp_sdivmul_result |= src1_op;
			tmp_sdivmul_result = tmp_sdivmul_result / (int64_t) ((int32_t) src2_op);
			dst_value = (uint32_t) (tmp_sdivmul_result & 0xffffffffL);
			sparc_cycle_counter += (CYCLES_DIV - CYCLES_INTEGER_INSTR);
			sparc_cycle_counter_local += (CYCLES_DIV - CYCLES_INTEGER_INSTR);
			break;
		default:
			break;
		}

	/* handle all instruction which influence icc of psr */
	switch (opcode) {
		/* all of the following instructions only check for
		   zero and negative */
		case ANDCC:
		case ANDNCC:
		case ORCC:
		case ORNCC:
		case XORCC:
		case XNORCC:
		case UMULCC:
		case SMULCC:
			changes_icc = 1;	
			/* clear next icc */
			next_icc = 0;
			if (dst_value & (1<<31)) { 
				PSR_SET_N(next_icc);
			}
			if (dst_value == 0) {
				PSR_SET_Z(next_icc);
			}
			break;
		case ADDCC:
		case TADDCC:
		case TADDCCTV:
			changes_icc = 1;
			/* clear next icc */
			next_icc = 0;
			if (dst_value & (1<<31)) {
				PSR_SET_N(next_icc);
			}
			if (dst_value == 0) {
				PSR_SET_Z(next_icc);
			}
			if ( ( (src1_op & (1<<31)) && (src2_op & (1<<31)) && (!(dst_value & (1<<31))) ) ||
				 ( (!(src1_op & (1<<31))) && (!(src2_op & (1<<31))) && (dst_value & (1<<31)) ) ) {
				PSR_SET_V(next_icc);
			}
			if ( ( (src1_op & (1<<31)) && (src2_op & (1<<31)) ) ||
				 ( (!(dst_value & (1<<31))) && ((src1_op & (1<<31)) || (src2_op & (1<<31))) ) ) {
				PSR_SET_C(next_icc);
			}
			break;
		case SUBCC:
		case TSUBCC:
		case TSUBCCTV:
			changes_icc = 1;
			/* clear next icc */
			next_icc = 0;
			if (dst_value & (1<<31)) {
				PSR_SET_N(next_icc);
			}
			if (dst_value == 0) {
				PSR_SET_Z(next_icc);
			}
			if ( ( (src1_op & (1<<31)) && (!(src2_op & (1<<31))) && (!(dst_value & (1<<31))) ) ||
				 ( (!(src1_op & (1<<31))) && (src2_op & (1<<31)) && (dst_value & (1<<31)) ) ) {
				PSR_SET_V(next_icc);
			}
			if ( ( (!(src1_op & (1<<31))) && (src2_op & (1<<31)) ) ||
				 ( (dst_value & (1<<31)) && ((!(src1_op & (1<<31))) || (src2_op & (1<<31))) ) ) {
				PSR_SET_C(next_icc);
			}
			break;
		case UDIVCC:
			changes_icc = 1;
			/* clear next icc */
			next_icc = 0;
			if (dst_value & (1<<31)) {
				PSR_SET_N(next_icc);
			}
			if (dst_value == 0) {
				PSR_SET_Z(next_icc);
			}
			if (tmp_udivmul_result & 0xffffffff00000000L) {
				PSR_SET_V(next_icc);
			}
		case SDIVCC:
			changes_icc = 1;
			/* clear next icc */
			next_icc = 0;
			if (dst_value & (1<<31)) {
				PSR_SET_N(next_icc);
			}
			if (dst_value == 0) {
				PSR_SET_Z(next_icc);
			}
			if ((tmp_sdivmul_result & 0xffffffff00000000L) && 
				((tmp_sdivmul_result >> 32) != 0xffffffffL)) {
				PSR_SET_V(next_icc);
			}
		default:
			break;
	}

	/* check whether the current instruction is predicated
	   or if we are within a predicated block... */
	if (sparc_pred_state.predicate_state == PREDICATE_STATE_NONE) {
		executed = 1;
	}
	/* predicated blocks on integer condition codes */
	if (sparc_pred_state.predicate_state == PREDICATE_STATE_ICC &&
		evaluateICC(sparc_pred_state.predicate_condition.icc)) {
		executed = 1;
	} 
	/* predicated blocks on predicate registers */
	if (sparc_pred_state.predicate_state == PREDICATE_STATE_PREG &&
		evaluatePred(sparc_pred_state.predicate_condition.preg_condition.preg, 
					sparc_pred_state.predicate_condition.preg_condition.tf)) {
		executed = 1;
	}

	if (executed) {
		/* handle load/store instructions */
		switch (opcode) {
			/* ldsba not implemented => same as normal ldsb */
			case LDSBA:
			case LDSB:
				if (dst_address) {
					/* get lower address */
					i = memory_address & 0x00000003;
					/* get byte according to address */
					dst_value >>= ((3 - i)*8); 
					dst_value &= 0x000000ff;
					/* sign extension */
					if (dst_value & 0x00000080) {
						dst_value |= 0xffffff00;
					}
					*dst_address = dst_value;
	/* 				fprintf(outstream, "ldsb [0x%08x]= 0x%08x\n", memory_address, dst_value); */
				}
				break;
			/* ldsha not implemented => same as normal ldsh */
			case LDSHA:
			case LDSH:
				if (dst_address) {
					/* check for valid address: LSB has to be zero! */
					if (memory_address & 0x00000001) {
						gen_simulator->cleanUp();
						simerror("Unknown memory address for ldsh instruction!");
					}
					/* get lower address */
					i = (memory_address & 0x00000002) >> 1;
					/* get halfword according to address */
					dst_value >>= ((1 - i)*16); 
					dst_value &= 0x0000ffff;
					/* sign extension */
					if (dst_value & 0x00008000) {
						dst_value |= 0xffff0000;
					}
					*dst_address = dst_value;
	/* 				fprintf(outstream, "ldsh [0x%08x]= 0x%08x\n", memory_address, dst_value); */
				}
				break;
			/* lduba not implemented => same as normal ldub */
			case LDUBA:
			case LDUB:
				if (dst_address) {
					/* get lower address */
					i = memory_address & 0x00000003;
					/* get byte according to address */
					dst_value >>= ((3 - i)*8); 
					dst_value &= 0x000000ff;
					*dst_address = dst_value;
	/* 				fprintf(outstream, "ldub [0x%08x]= 0x%08x\n", memory_address, dst_value); */
				}
				break;
			/* lduha not implemented => same as normal lduh */
			case LDUHA:
			case LDUH:
				if (dst_address) {
					/* check for valid address: LSB has to be zero! */
					if (memory_address & 0x00000001) {
						gen_simulator->cleanUp();
						simerror("Unknown memory address for lduh instruction!");
					}
					/* get lower address */
					i = (memory_address & 0x00000002) >> 1;
					/* get halfword according to address */
					dst_value >>= ((1 - i)*16); 
					dst_value &= 0x0000ffff;
					*dst_address = dst_value;
				}
				break;
			/* lda not implemented => same as normal ld */
			case LDA:
			case LD:
				if (dst_address) {
					/* check address to be valid */
					if (memory_address & 0x00000003) {
						gen_simulator->cleanUp();
						simerror("Unknown memory address for ld instruction!");
					}
					
					*dst_address = dst_value;
				}
				break;
			case LDDA:
			case LDD:
				fprintf(stderr, "Warning: simulator currently does not "
					"implement load double instructions!\n");
				break;
			case STBA:
			case STB:
				data_memory[memory_address] = (uint8_t) (dst_value & (0x000000ff));
				break;
			case STHA:
			case STH:
				/* check address to be valid */
				if (memory_address & 0x00000001) {
					gen_simulator->cleanUp();
					simerror("Unknown destination address for sth instruction!");
				}
				for (i = 1; i >= 0; i--) {
					data_memory[(memory_address & 0xfffffffe) + i] = 
						(uint8_t) (dst_value & (0x000000ff));
					dst_value >>= 8;
				}
				break;
			case STA:
			case ST:
				/* check address to be valid */
				if (memory_address & 0x00000003) {
					gen_simulator->cleanUp();
					simerror("Unknown destination address for st instruction!");
				}
				for (i = 3; i >= 0; i--) {
					data_memory[(memory_address & 0xfffffffc) + i] = 
						(uint8_t) (dst_value & (0x000000ff));
					dst_value >>= 8;
				}
				break;
			case STDA:
			case STD:
				fprintf(stderr, "Warning: simulator currently does not "
					"implement store double instructions!\n");
				break;
			case JUMPL:
				sparc_npc = memory_address;
				if (dst_address) {
					*dst_address = (cur_pc << 2);
				}
				break;
			/* handle predset/predclear instructions */
			case PREDSET:
			case PREDCLEAR:
				sparc_preg = next_preg;
				break;
			/* write y register in case of a multiplication */
			case UMUL:
			case UMULCC:
			case SMUL:
			case SMULCC:
				sparc_y = tmp_y_value;
			default:
				if (dst_address) {
					*dst_address = dst_value;
				}
				break;
		}
		/* if the current instruction influences the icc... */
		if (changes_icc) {
			PSR_CLR_ICCS(sparc_psr);
			sparc_psr |= next_icc;
		}

	}

	/* if the next instruction is end of memory => return from main... */
	if (sparc_pc == (END_OF_INS_MEM>>2)) {
		return 0;
	} else {
		return 1;
	}

}

/**
  * @brief Prints the return value of the main function and the
  *        number of simulated cycles to the given file stream.
  * @param[in] outstream File stream where to print the information.
  */
void printResults(FILE* outstream) {
	fprintf(outstream, "Main function returned value 0x%08x.\n", 
		*(sparc_window_registers[RET_VAL_REGISTER]));
	fprintf(outstream, "Simulated cycles: %d.\n", sparc_cycle_counter);
}

/**
  * @brief Prints the register contents of the current window,
  *        the ICC flags of the psr, the contents of the preg and
  *        the y-register to the given file stream.
  * @param[in] outstream File stream where to print the information.
  */
void printRegisters(FILE* outstream) {

	uint32_t i;

	/* register names */
	const char* reg_names = "goli";

	fprintf(outstream, "Register contents of current window:\n");
	for(i = 0; i < 32; i++) {
		fprintf(outstream, "%%%c%d:\t0x%08x\n", reg_names[(i/8)], 
			(i%8), *(sparc_window_registers[i]));
	}
	fprintf(outstream, "%%y:\t\t0x%08x\n", sparc_y);
	fprintf(outstream, "PSR:\tN=%d, Z=%d, V=%d, C=%d\n", 
		PSR_GET_N(sparc_psr), PSR_GET_Z(sparc_psr), 
		PSR_GET_V(sparc_psr), PSR_GET_C(sparc_psr));
	fprintf(outstream, "\n");
	fprintf(outstream, "preg:\t0x%08x\n", sparc_preg);

}

/**
  * @brief Prints out all instructions on the given output file stream. 
  * @param[in] outstream File stream where to print the information.
  */
void printInstructions(FILE* outstream) {

	uint32_t i;
	uint32_t number_instructions = gen_simulator->getNumberOfInstructions();
	
	uint32_t operand_iter;
	uint32_t num_operands, operands_end;
	int opcode;

	int reg;
	int immediate;
	int icc;
	uint32_t address;

	/* register names */
	const char* reg_names = "goli";

	/* branch codes in plain text */
	static const char* branch_codes[16] = {
		"n", "e", "le", "l", "leu", "cs",
		"neg", "vs", "a", "ne", "g", "ge",
		"gu", "cc", "pos", "vc" 
	};


	fprintf(outstream, "Contents of instruction memory (%d bytes):\n", 
		header.instruction_size);

	/* iterate over all existing instructions */
	for (i = 0; i < number_instructions; i++) {

		opcode = instructions[i].opcode;
		num_operands = instructions[i].num_operands;
		operands_end = num_operands;
		operand_iter = 0;
		
		fprintf(outstream, "%08x\t", i);

		switch(opcode) {
			case CYCLE_PRINT:
				/* only meta instruction for simulator */
				fprintf(outstream, "sim-printcycles");
				break;
			case CYCLE_CLEAR:
				/* only meta instruction for simulator */
				fprintf(outstream, "sim-clearcycles");
				break;
			case CALL:
				address = instructions[i].operands[0].value.labeladdress;
				fprintf(outstream, "call 0x%08x", address);
				/* there are no unhandled operands */
				operands_end = 0;
				break;
			case SETHI:
				fprintf(outstream, "sethi");
				break;
			case NOP:
				fprintf(outstream, "nop");
				break;
			case BRANCH:
				address = instructions[i].operands[0].value.labeladdress;
				icc = instructions[i].operands[1].value.icc;
				fprintf(outstream, "b%s", branch_codes[icc]);
				fprintf(outstream, " 0x%08x", address);
				/* there are no unhandled operands */
				operands_end = 0;
				break;
			case LDSB:
				fprintf(outstream, "ldsb");
				break;
			case LDSH:
				fprintf(outstream, "ldsh");
				break;
			case LDUB:
				fprintf(outstream, "ldub");
				break;
			case LDUH:
				fprintf(outstream, "lduh");
				break;
			case LD:
				fprintf(outstream, "ld");
				break;
			case LDD:
				fprintf(outstream, "ldd");
				break;
			case LDSBA:
				fprintf(outstream, "ldsba");
				break;
			case LDSHA:
				fprintf(outstream, "ldsha");
				break;
			case LDUBA:
				fprintf(outstream, "lduba");
				break;
			case LDUHA:
				fprintf(outstream, "lduha");
				break;
			case LDA:
				fprintf(outstream, "lda");
				break;
			case LDDA:
				fprintf(outstream, "ldda");
				break;
			case STB:
				fprintf(outstream, "stb");
				break;
			case STH:
				fprintf(outstream, "sth");
				break;
			case ST:
				fprintf(outstream, "st");
				break;
			case STD:
				fprintf(outstream, "std");
				break;
			case STBA:
				fprintf(outstream, "stba");
				break;
			case STHA:
				fprintf(outstream, "stha");
				break;
			case STA:
				fprintf(outstream, "sta");
				break;
			case STDA:
				fprintf(outstream, "stda");
				break;
			case LDSTUB:
				fprintf(outstream, "ldstub");
				break;
			case LDSTUBA:
				fprintf(outstream, "ldstuba");
				break;
			case SWAP:
				fprintf(outstream, "swap");
				break;
			case SWAPA:
				fprintf(outstream, "swapa");
				break;
			case AND:
				fprintf(outstream, "and");
				break;
			case ANDCC:
				fprintf(outstream, "andcc");
				break;
			case ANDN:
				fprintf(outstream, "andn");
				break;
			case ANDNCC:
				fprintf(outstream, "andncc");
				break;
			case OR:
				fprintf(outstream, "or");
				break;
			case ORCC:
				fprintf(outstream, "orcc");
				break;
			case ORN:
				fprintf(outstream, "orn");
				break;
			case ORNCC:
				fprintf(outstream, "orncc");
				break;
			case XOR:
				fprintf(outstream, "xor");
				break;
			case XORCC:
				fprintf(outstream, "xorcc");
				break;
			case XNOR:
				fprintf(outstream, "xnor");
				break;
			case XNORCC:
				fprintf(outstream, "xnorcc");
				break;
			case SLL:
				fprintf(outstream, "sll");
				break;
			case SRL:
				fprintf(outstream, "srl");
				break;
			case SRA:
				fprintf(outstream, "sra");
				break;
			case ADD:
				fprintf(outstream, "add");
				break;
			case ADDCC:
				fprintf(outstream, "addcc");
				break;
			case ADDX:
				fprintf(outstream, "addx");
				break;
			case ADDXCC:
				fprintf(outstream, "addxcc");
				break;
			case TADDCC:
				fprintf(outstream, "taddcc");
				break;
			case TADDCCTV:
				fprintf(outstream, "taddcctv");
				break;
			case SUB:
				fprintf(outstream, "sub");
				break;
			case SUBCC:
				fprintf(outstream, "subcc");
				break;
			case SUBX:
				fprintf(outstream, "subx");
				break;
			case SUBXCC:
				fprintf(outstream, "subxcc");
				break;
			case TSUBCC:
				fprintf(outstream, "tsubcc");
				break;
			case TSUBCCTV:
				fprintf(outstream, "tsubcctv");
				break;
			case MULSCC:
				fprintf(outstream, "mulscc");
				break;
			case UMUL:
				fprintf(outstream, "umul");
				break;
			case SMUL:
				fprintf(outstream, "smul");
				break;
			case UMULCC:
				fprintf(outstream, "umulcc");
				break;
			case SMULCC:
				fprintf(outstream, "smulcc");
				break;
			case UDIV:
				fprintf(outstream, "udiv");
				break;
			case SDIV:
				fprintf(outstream, "sdiv");
				break;
			case UDIVCC:
				fprintf(outstream, "udivcc");
				break;
			case SDIVCC:
				fprintf(outstream, "sdivcc");
				break;
			case SAVE:
				fprintf(outstream, "save");
				break;
			case RESTORE:
				fprintf(outstream, "restore");
				break;
			case JUMPL:
				fprintf(outstream, "jumpl");
				break;
			case RD:
				fprintf(outstream, "rd");
				/* handle predicated rd instruction */
				if (num_operands == 3) {
					icc = instructions[i].operands[2].value.icc;
					fprintf(outstream, "[%s]", branch_codes[icc]);
				} else if (num_operands == 4) {
					reg = instructions[i].operands[2].value.preg;
					fprintf(outstream, "[%%p%d]", reg);
					if (instructions[i].operands[3].value.tf) {
						fprintf(outstream, "[t]");
					} else {
						fprintf(outstream, "[f]");
					}
				}
				reg = instructions[i].operands[1].value.reg;
				if (reg != Y_REGISTER_NO) {
					cleanUp();
					simerror("Unknown source register for read instruction!");
				}
				fprintf(outstream, " %%y,");
				/* there are no unhandled operands */
				operands_end = 0;
				break;
			case WR:
				fprintf(outstream, "wr");
				break;
			case MOV:
				icc = instructions[i].operands[2].value.icc;
				fprintf(outstream, "mov[%s]", branch_codes[icc]);
				/* last operand has already been printed */
				operands_end = 2;
				break;
			case SEL:
				icc = instructions[i].operands[3].value.icc;
				fprintf(outstream, "sel[%s]", branch_codes[icc]);
				/* select needs special treatment for src operands */
				if (instructions[i].operands[1].type == OPERAND_TYPE_REGISTER) {
					reg = instructions[i].operands[1].value.reg;
					fprintf(outstream, " %%%c%d,", reg_names[(reg/8)], (reg%8));
					if (instructions[i].operands[2].type == OPERAND_TYPE_REGISTER) {
						reg = instructions[i].operands[2].value.reg;
						fprintf(outstream, " %%%c%d,", reg_names[(reg/8)], (reg%8));
					} else if (instructions[i].operands[2].type == OPERAND_TYPE_SIMM11) {
						immediate = instructions[i].operands[2].value.simm11;
						fprintf(outstream, " %d,", immediate);
					} else {
						cleanUp();
						simerror("Unknown type for source2 of selcc instruction!");
					}

				} else if (instructions[i].operands[1].type == OPERAND_TYPE_SIMM8) {
					immediate = instructions[i].operands[1].value.simm8;
					fprintf(outstream, " %d,", immediate);
					/* second operand has to be simm8 then... */
					immediate = instructions[i].operands[2].value.simm8;
					fprintf(outstream, " %d,", immediate);
				} else {
					cleanUp();
					simerror("Unknown type for source1 of selcc instruction!");
				}

				/* all operands have already been printed */
				operands_end = 0;
				break;
			case HWLOOP_INIT:
				fprintf(outstream, "hwloop init ");
				address = instructions[i].operands[1].value.labeladdress;
				reg = instructions[i].operands[0].value.loopreg;
				if (reg == LOOPS_REGISTER) {
					fprintf(outstream, "0x%08x, %%loops", address);
				} else if (reg == LOOPE_REGISTER) {
					fprintf(outstream, "0x%08x, %%loope", address);
				} else if (reg == LOOPB_REGISTER) {
					if (instructions[i].operands[1].type == OPERAND_TYPE_REGISTER) {
						reg = instructions[i].operands[1].value.reg;
						fprintf(outstream, "%%%c%d, ", reg_names[reg/8], (reg%8));
					} else {
						immediate = instructions[i].operands[1].value.imm22;
						fprintf(outstream, "%d, ", immediate);
					}
					fprintf(outstream, "%%loopb");
				}
				operands_end = 0;
				break;
			case HWLOOP_START:
				fprintf(outstream, "hwloop start");
				operands_end = 0;
				break;
			case PREDBEGIN:
				fprintf(outstream, "predbegin");
				if (instructions[i].operands[0].type == OPERAND_TYPE_ICC) {
					icc = instructions[i].operands[0].value.icc;
					fprintf(outstream, "[%s]", branch_codes[icc]);
				} else {
					reg = instructions[i].operands[0].value.preg;
					fprintf(outstream, "[%%p%d]", reg);
					if (instructions[i].operands[1].value.tf) {
						fprintf(outstream, "[t]");
					} else {
						fprintf(outstream, "[f]");
					}
				}
				/* there are no unhandled operands */
				operands_end = 0;
				break;
			case PREDEND:
				fprintf(outstream, "predend");
				break;
			case PREDSET:
				fprintf(outstream, "predset");
				/* handle predicated versions of predset */
				if (num_operands > 1) {
					if (instructions[i].operands[1].type == OPERAND_TYPE_ICC) {
						icc = instructions[i].operands[1].value.icc;
					} else {
						reg = instructions[i].operands[1].value.reg;
						fprintf(outstream, "[%%p%d]", reg);
						if (instructions[i].operands[2].value.tf) {
							fprintf(outstream, "[t]");
						} else {
							fprintf(outstream, "[f]");
						}
						icc = instructions[i].operands[3].value.icc;
					}
					fprintf(outstream, "[%s]", branch_codes[icc]);
				}
				if (instructions[i].operands[0].type != OPERAND_TYPE_PREG) {
					cleanUp();
					simerror("Unknown register type for predclear instruction!");
				}
				reg = instructions[i].operands[0].value.preg;
				fprintf(outstream, " %%p%d", reg);
				/* there are no unhandled operands */
				operands_end = 0;
				break;
			case PREDCLEAR:
				if (instructions[i].operands[0].type != OPERAND_TYPE_PREG) {
					cleanUp();
					simerror("Unknown register type for predclear instruction!");
				}
				reg = instructions[i].operands[0].value.preg;
				fprintf(outstream, "predclear %%p%d", reg);
				/* there are no unhandled operands */
				operands_end = 0;
				break;
			case UNKNOWN: 
			default:
				cleanUp();
				simerror("Not supported opcode encoutered!");
				break;
		}
		/* fully predicated instructions */
		if (operands_end == 5) {
			reg = instructions[i].operands[3].value.preg;
			fprintf(outstream, "[%%p%d]", reg);
			if (instructions[i].operands[4].value.tf) {
				fprintf(outstream, "[t]");
			} else {
				fprintf(outstream, "[f]");
			}
			operands_end = 3;
		} else if (operands_end == 4) {
		/* predicated instructions on codition code */
			icc = instructions[i].operands[3].value.icc;
			fprintf(outstream, "[%s]", branch_codes[icc]);
			operands_end = 3;
		}

		for (operand_iter = 1; operand_iter < operands_end; operand_iter++) {
			if (instructions[i].operands[operand_iter].type == OPERAND_TYPE_REGISTER) {
				reg = instructions[i].operands[operand_iter].value.reg;
				fprintf(outstream, " %%%c%d", reg_names[(reg/8)], (reg%8));
			} else if (instructions[i].operands[operand_iter].type == OPERAND_TYPE_SIMM13) {
				immediate = instructions[i].operands[operand_iter].value.simm13;
				fprintf(outstream, " %d", immediate);
			} else if (instructions[i].operands[operand_iter].type == OPERAND_TYPE_IMM22) {
				immediate = instructions[i].operands[operand_iter].value.imm22;
				fprintf(outstream, " %d", immediate);
			}
			fprintf(outstream, ",");
		}
		if ((num_operands > 0) && instructions[i].operands[0].type == OPERAND_TYPE_REGISTER) {
			reg = instructions[i].operands[0].value.reg;
			if (opcode != WR) {
				fprintf(outstream, " %%%c%d", reg_names[(reg/8)], (reg%8));
			} else {
				fprintf(outstream, " %%y");
			}
		}
		fprintf(outstream, "\n");
	}
	fprintf(outstream, "\n");
}

/**
  * @brief Prints out the contents of the data memory to the given
  *        output file stream.
  * @param[in] outstream File stream where to print the information.
  */
void printMemory(FILE* outstream) {

	uint32_t i;

	fprintf(outstream, "Contents of data memory (%d bytes):\n", 
		data_memory_size);
	for (i = 0; i < data_memory_size; i++) {
		if ((i%16) == 0) {
			fprintf(outstream, "%08x\t", i);
		}
		fprintf(outstream, "%02x", data_memory[i]);
		if ((i%4) == 3) {
			fprintf(outstream, " ");
		}
		if ((i%16) == 15) {
			fprintf(outstream, "\n");
		}
	}
	fprintf(outstream, "\n\n");

}

/**
  * @brief Returns a pointer to the simulator header struct.
  * @return Pointer to the simulator header struct.
  */
simulator_header_t* getFileHeader(void) {
	return &header;
}

/**
  * @brief Returns the address of the pointer to the instruction
  *        array.
  * @return Address of the pointer to the instruction array.
  */
sparc_instruction** getInstructions(void) {
	return &instructions;
}

/**
  * @brief Registers generic simulator functions.
  * @param[in,out] simulator The generic simulator data structure.
  * @param[in] error_fct The error function of the simulator 
  *                      which terminates the program.
  * @return 0 on success, 1 otherwise.
  */
int gen_simulator_init(gen_simulator_t* simulator, error_fct_t error_fct) {
	
	simulator->readFileHeader = readFileHeader;
	simulator->readMemory = readMemory;

	simulator->printInstructions = printInstructions;
	simulator->printMemory = printMemory;
	simulator->printRegisters = printRegisters;
	simulator->printResults = printResults;
	
	simulator->resetSimulator = resetSimulator;

	simulator->simulateStep = simulateStep;

	simulator->getInstructions = getInstructions;
	simulator->getFileHeader = getFileHeader;
	
	simulator->cleanUp = cleanUp;

	simerror = error_fct;
	gen_simulator = simulator;

	return 0;

}
