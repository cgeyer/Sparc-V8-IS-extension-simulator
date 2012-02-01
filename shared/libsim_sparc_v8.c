/*
 * SPARC V8 Instruction Set Extension Simulator
 *
 * File: shared/libsim_sparc_v8.c
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

#include "gen_simulator.h"
#include "sparc.tab.h"
#include "sparc_target.h"

#include "sparc_v8.h"

/** Pointer to the generic simulator object. */
static gen_simulator_t* gen_simulator;
/** Save the number of instructions for cleanUp function. */
static uint32_t number_instructions;

/** Pointer to error function of simulator. */
static error_fct_t simerror;

/**
  * @brief Converts the target specific opcode into the generic
  * opcode of the simulator.
  * @return The internal opcode for the generic simulator 
  * instruction.
  */
static int getOpcode(uint32_t opcode) {

	int op;
	int op2;
	int op3;
	int return_opcode = UNKNOWN;

	op = GET_OP(opcode);

	/* fprintf(stderr, "0x%08x\n", opcode); */

	switch (op) {
		
		/* only call instruction has format 1*/
		case 1:
			return_opcode = CALL;
			break;
		
		/* check, whether we have branch or sethi instructions */
		case 0:
			op2 = GET_OP2(opcode);
			switch (op2) {
				case OP2_BICC:
					return_opcode = BRANCH;
					break;
				case OP2_SETHI:
					/* don't forget to check for NOP later!*/
					return_opcode = SETHI;
					break;
				case OP2_SIMCYCLES:
					if (GET_RD(opcode) == SIM_CYCLES_PRINT) {
						return_opcode = CYCLE_PRINT; 
					} else if (GET_RD(opcode) == SIM_CYCLES_CLEAR) {
						return_opcode = CYCLE_CLEAR;
					}
					break;
			}
			break;

		/* check for logical and arithmetic instructions */
		case 2:
			op3 = GET_OP3(opcode);
			/* fprintf(stderr, "logical/arithmetic instruction - op3 = 0x%02x\n", op3); */
			switch (op3) {
				case OP3_AND:
					return_opcode = AND;
					break;
				case OP3_ANDCC:
					return_opcode = ANDCC;
					break;
				case OP3_ANDN:
					return_opcode = ANDN;
					break;
				case OP3_ANDNCC:
					return_opcode = ANDNCC;
					break;
				case OP3_OR:
					return_opcode = OR;
					break;
				case OP3_ORCC:
					return_opcode = ORCC;
					break;
				case OP3_ORN:
					return_opcode = ORN;
					break;
				case OP3_ORNCC:
					return_opcode = ORNCC;
					break;
				case OP3_XOR:
					return_opcode = XOR;
					break;
				case OP3_XORCC:
					return_opcode = XORCC;
					break;
				case OP3_XNOR:
					return_opcode = XNOR;
					break;
				case OP3_XNORCC:
					return_opcode = XNORCC;
					break;
				case OP3_SLL:
					return_opcode = SLL;
					break;
				case OP3_SRL:
					return_opcode = SRL;
					break;
				case OP3_SRA:
					return_opcode = SRA;
					break;
				case OP3_ADD:
					return_opcode = ADD;
					break;
				case OP3_ADDCC:
					return_opcode = ADDCC;
					break;
				case OP3_ADDX:
					return_opcode = ADDX;
					break;
				case OP3_ADDXCC:
					return_opcode = ADDXCC;
					break;
				case OP3_TADDCC:
					return_opcode = TADDCC;
					break;
				case OP3_TADDCCTV:
					return_opcode = TADDCCTV;
					break;
				case OP3_SUB:
					return_opcode = SUB;
					break;
				case OP3_SUBCC:
					return_opcode = SUBCC;
					break;
				case OP3_SUBX:
					return_opcode = SUBX;
					break;
				case OP3_SUBXCC:
					return_opcode = SUBXCC;
					break;
				case OP3_TSUBCC:
					return_opcode = TSUBCC;
					break;
				case OP3_TSUBCCTV:
					return_opcode = TSUBCCTV;
					break;
				case OP3_MULSCC:
					return_opcode = MULSCC;
					break;
				case OP3_UMUL:
					return_opcode = UMUL;
					break;
				case OP3_SMUL:
					return_opcode = SMUL;
					break;
				case OP3_UMULCC:
					return_opcode = UMULCC;
					break;
				case OP3_SMULCC:
					return_opcode = SMULCC;
					break;
				case OP3_UDIV:
					return_opcode = UDIV;
					break;
				case OP3_SDIV:
					return_opcode = SDIV;
					break;
				case OP3_UDIVCC:
					return_opcode = UDIVCC;
					break;
				case OP3_SDIVCC:
					return_opcode = SDIVCC;
					break;
				case OP3_SAVE:
					return_opcode = SAVE;
					break;
				case OP3_RESTORE:
					return_opcode = RESTORE;
					break;
				case OP3_JUMPL:
					return_opcode = JUMPL;
					break;
				case OP3_RDY:
					return_opcode = RD;
					break;
				case OP3_WRY:
					return_opcode = WR;
					break;
			}
			break;

		/* check for memory instructions */
		case 3:
			op3 = GET_OP3(opcode);
			switch (op3) {
				case OP3_LDSB:
					return_opcode = LDSB;
					break;
				case OP3_LDSH:
					return_opcode = LDSH;
					break;
				case OP3_LDUB:
					return_opcode = LDUB;
					break;
				case OP3_LDUH:
					return_opcode = LDUH;
					break;
				case OP3_LD:
					return_opcode = LD;
					break;
				case OP3_LDD:
					return_opcode = LDD;
					break;
				case OP3_LDSBA:
					return_opcode = LDSBA;
					break;
				case OP3_LDSHA:
					return_opcode = LDSHA;
					break;
				case OP3_LDUBA:
					return_opcode = LDUBA;
					break;
				case OP3_LDUHA:
					return_opcode = LDUHA;
					break;
				case OP3_LDA:
					return_opcode = LDA;
					break;
				case OP3_LDDA:
					return_opcode = LDDA;
					break;
				case OP3_STB:
					return_opcode = STB;
					break;
				case OP3_STH:
					return_opcode = STH;
					break;
				case OP3_ST:
					return_opcode = ST;
					break;
				case OP3_STD:
					return_opcode = STD;
					break;
				case OP3_STBA:
					return_opcode = STBA;
					break;
				case OP3_STHA:
					return_opcode = STHA;
					break;
				case OP3_STA:
					return_opcode = STA;
					break;
				case OP3_STDA:
					return_opcode = STDA;
					break;
				case OP3_LDSTUB:
					return_opcode = LDSTUB;
					break;
				case OP3_LDSTUBA:
					return_opcode = LDSTUBA;
					break;
				case OP3_SWAP:
					return_opcode = SWAP;
					break;
				case OP3_SWAPA:
					return_opcode = SWAPA;
					break;
			}
			break;
	}

	return return_opcode;

}

/**
  * @brief Saves the correct opcode and all operands of the current instruction.
  * @param[in] opcode The binary target specific opcode.
  * @param[in,out] instruction The instruction which shall be saved.
  */
static void saveInstruction(uint32_t opcode, sparc_instruction* instruction) {

	int sim_opcode = getOpcode(opcode);
	int dst_reg;
	int src1_reg;
	int src2_reg;
	int immediate;
	int icc;

	static char errormsg[100]; 

	/* save opcode */
	instruction->opcode = sim_opcode;

	/* save operands */
	switch (sim_opcode) {

		/* calls only have a displacement: */
		case CALL:
		
			immediate = GET_DISP30(opcode);
			/* sign extension of displacement */
			if (immediate & (1<<29)) {
				immediate |= ((1<<30)|(1<<31));
			}
			/* save call displacement as operand */
			instruction->num_operands = 1;
			instruction->operands = malloc(sizeof(sparc_operand));

			/* memory allocation OK? */
			if (!(instruction->operands)) {
				gen_simulator->cleanUp();
				simerror("Could not allocate memory for instruction operands!");
			}

			instruction->operands[0].type = OPERAND_TYPE_LABEL_ADDRESS;
			/* label address is (absolute) instruction number */
			instruction->operands[0].value.labeladdress = (unsigned) ((int) instruction->instr_no + immediate);
			break;
		
		/* handle branch instructions */
		case BRANCH:
			immediate = GET_IMM22(opcode);
			/* sign extension of displacement */ 
			if (immediate & (1<<21)) {
				immediate |= 0xFFC00000; 
			}
			icc = GET_CC(opcode);

			instruction->num_operands = 2;
			instruction->operands = malloc(sizeof(sparc_operand)*2);

			/* memory allocation OK? */
			if (!(instruction->operands)) {
				gen_simulator->cleanUp();
				simerror("Could not allocate memory for instruction operands!");
			}
			
			instruction->operands[0].type = OPERAND_TYPE_LABEL_ADDRESS;
			/* label address is (absolute) instruction number */
			instruction->operands[0].value.labeladdress = (unsigned) ((int) instruction->instr_no + immediate);

			/* save integer condition code */
			instruction->operands[1].type = OPERAND_TYPE_ICC;
			instruction->operands[1].value.icc = icc;

			break;

		/* handle sethi and nop instructions */
		case SETHI:
			immediate = GET_IMM22(opcode);
			dst_reg = GET_RD(opcode);

			/* if destination register and immediate are zero, it's a NOP instruction*/
			if (!dst_reg && !immediate) {

				instruction->num_operands = 0;
				instruction->opcode = NOP;

			} else {

				instruction->num_operands = 2;

				instruction->operands = malloc(sizeof(sparc_operand)*2);

				/* memory allocation OK? */
				if (!(instruction->operands)) {
					gen_simulator->cleanUp();
					simerror("Could not allocate memory for instruction operands!");
				}

				instruction->operands[0].type = OPERAND_TYPE_REGISTER;
				instruction->operands[0].value.reg = dst_reg;

				instruction->operands[1].type = OPERAND_TYPE_IMM22;
				instruction->operands[1].value.imm22 = immediate;

			}
			break;

		case RD:
			dst_reg = GET_RD(opcode);
			src1_reg = GET_RS1(opcode);
			if (src1_reg != Y_REGISTER_NO) {
				gen_simulator->cleanUp();
				simerror("Unknown source register for rd instruction!");
			}

			instruction->num_operands = 2;

			instruction->operands = malloc(sizeof(sparc_operand)*2);

			/* memory allocation OK? */
			if (!(instruction->operands)) {
				gen_simulator->cleanUp();
				simerror("Could not allocate memory for instruction operands!");
			}

			instruction->operands[0].type = OPERAND_TYPE_REGISTER;
			instruction->operands[0].value.reg = dst_reg;

			instruction->operands[1].type = OPERAND_TYPE_REGISTER;
			instruction->operands[1].value.reg = src1_reg;
			break;

		/* nothing to do for sim-cylce instruction */
		case CYCLE_PRINT:
		case CYCLE_CLEAR:
			instruction->num_operands = 0;
			break;

		/* terminate on unkown instruction */
		case UNKNOWN:
			gen_simulator->cleanUp();
			snprintf(errormsg, 100, "Encountered unknown opcode at instruction no %d!", 
				instruction->instr_no);
			simerror(errormsg);
			break;

		/* all other instructions have exactly three operands */
		default:

			dst_reg = GET_RD(opcode);
			src1_reg = GET_RS1(opcode);

			instruction->num_operands = 3;			
			
			instruction->operands = malloc(sizeof(sparc_operand)*3);

			/* memory allocation OK? */
			if (!(instruction->operands)) {
				gen_simulator->cleanUp();
				simerror("Could not allocate memory for instruction operands!");
			}

			instruction->operands[0].type = OPERAND_TYPE_REGISTER;
			instruction->operands[0].value.reg = dst_reg;

			instruction->operands[1].type = OPERAND_TYPE_REGISTER;
			instruction->operands[1].value.reg = src1_reg;

			/* is third operand an immediate? */
			if (GET_I(opcode)) {
				immediate = GET_SIMM13(opcode);
				/* sign extension of immediate */
				if (immediate & (1<<12)) {
					immediate |= 0xFFFFE000;
				}
				instruction->operands[2].type = OPERAND_TYPE_SIMM13;
				instruction->operands[2].value.simm13 = immediate;
			} else {
				src2_reg = GET_RS2(opcode);
				instruction->operands[2].type = OPERAND_TYPE_REGISTER;
				instruction->operands[2].value.reg = src2_reg;
			}
			break;

	}


}

/**
  * @brief Checks, whether the current file is supported by the specific target.
  * @return 0 on success, 1 otherwise
  */
int checkTargetID(void) {
	if (gen_simulator->getFileHeader()->target_id != TARGET_ID) {
		return 1;
	} else {
		return 0;
	}
}


/**
  * @brief Reads in target specific instructions from the given file 
  * and generates generic instruction for the simulator.
  * @param[in] instream File containing the instructions in binary format.
  */
void readInstructions(FILE* instream) {
	
	uint32_t instruction_size = gen_simulator->getFileHeader()->instruction_size;

	sparc_instruction** instructions = gen_simulator->getInstructions();
	sparc_instruction* instructions_array;

	uint32_t opcode;

	uint32_t i, j;
	int value;

	/* all binary instructions have 4 bytes */
	if (instruction_size % 4) {
		gen_simulator->cleanUp();
		simerror("Invalid instruction size: has to be a multiple of 4!");
	} else {
		instruction_size = instruction_size / 4;
	}

	/* allocate memory for instructions */
	*instructions = malloc(sizeof(sparc_instruction)*(instruction_size));
	instructions_array = *instructions;

	if (!instructions_array) {
		gen_simulator->cleanUp();
		simerror("Could not allocate memory for instructions!");
	}

	/* set number of allocated instructions */
	number_instructions = instruction_size;

	/* read in instructions */
	for (i = 0; i < instruction_size; i++) {

		/* clear current opcode */
		opcode = 0;

		/* read in opcode from current filestream */
		for (j = 0; j < 4; j++) {
			if ((value = fgetc(instream)) == EOF) {
				gen_simulator->cleanUp();
				simerror("Could not read from file!");
			}
			opcode <<= 8;
			opcode |= (uint32_t) ((value) & 0xff);
		}

		/* save instruction number */
		instructions_array[i].instr_no = i;

		/* convert opcode to instruction data structure */
		saveInstruction(opcode, &(instructions_array[i]));

	}

}

/**
  * @brief Returns the number of allocated instructions.
  * @return Number of allocated instructions.
  */
uint32_t getNumberOfInstructions(void) {
	return number_instructions;
}

/**
  * @brief Registers target specific simulator functions.
  * @param[in,out] simulator The generic simulator data structure.
  * @param[in] error_fct The error function of the simulator 
  * which terminates the program.
  * @return 0 on success, 1 otherwise.
  */
int simulator_init(gen_simulator_t* simulator, error_fct_t error_fct) {

	simulator->readInstructions = readInstructions;
	simulator->checkTargetID = checkTargetID;
	simulator->getNumberOfInstructions = getNumberOfInstructions;

	simerror = error_fct;
	gen_simulator = simulator;
	
	return 0;

}

