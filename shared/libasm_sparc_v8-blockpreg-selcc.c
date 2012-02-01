/*
 * SPARC V8 Instruction Set Extension Simulator
 *
 * File: shared/libasm_sparc_v8-blockpreg-selcc.c
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
#include <stdlib.h>
#include <string.h>

#include <stdint.h>

#include "sparc_v8-blockpreg-selcc.h"
#include "gen_assembler.h"
#include "sparc_target.h"
#include "sparc.tab.h"

/** Pointer to generic assembler object. */
gen_assembler_t* gen_assembler;

/**
  * @brief Sets all needed bits of the target sepcific binary
  * instruction.
  * @param[out] bin_instruction Pointer to the binary instruction.
  * @param[in] opcode The generic opcode of the assembler.
  */
static void init_bin_instruction(uint32_t* bin_instruction, int opcode) {

	switch(opcode) {
		/* call is the only format 1 instruction */
		case CALL:
			FORMAT_1(*bin_instruction);
			return;
			break;
		/* sethi/nop and bicc are the only (supported) format 2 instructions */
		case SETHI:
		case NOP:
			FORMAT_2(*bin_instruction);
			SET_OP2(*bin_instruction, OP2_SETHI);
			return;
			break;
		case BRANCH:
			FORMAT_2(*bin_instruction);
			SET_OP2(*bin_instruction, OP2_BICC);
			return;
			break;
		/* handle meta instructions for simulator */
		case CYCLE_PRINT:
			FORMAT_2(*bin_instruction);
			SET_OP2(*bin_instruction, OP2_SIMCYCLES);
			SET_RD(*bin_instruction, SIM_CYCLES_PRINT);
			return;
			break;
		case CYCLE_CLEAR:
			FORMAT_2(*bin_instruction);
			SET_OP2(*bin_instruction, OP2_SIMCYCLES);
			SET_RD(*bin_instruction, SIM_CYCLES_CLEAR);
			return;
			break;
		/* target specific HWLoop instruction */
		case HWLOOP_START:
		case HWLOOP_INIT:
			FORMAT_2(*bin_instruction);
			SET_OP2(*bin_instruction, OP2_HWLOOP);
			return;
			break;
		/* target specific SELcc instruction */
		case SEL:
			FORMAT_2(*bin_instruction);
			SET_OP2(*bin_instruction, OP2_SELCC);
			return;
			break;
		/* target specific predicated block instructions on preg */
		case PREDBEGIN:
		case PREDEND:
		case PREDSET:
		case PREDCLEAR:
			FORMAT_2(*bin_instruction);
			SET_OP2(*bin_instruction, OP2_PREDBLOCKSPREG);
			return;
			break;
		/* all memory instructions have the same format 3 encoding */
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
		case LDA:
		case LDDA:
		case STB:
		case STH:
		case ST:
		case STD:
		case STBA:
		case STHA:
		case STA:
		case STDA:
		case LDSTUB:
		case LDSTUBA:
		case SWAP:
		case SWAPA:
			FORMAT_3_mem(*bin_instruction);
			break;
		/* all remaining instructions have format 3 */
		default:
			FORMAT_3_oth(*bin_instruction);
			break;			
	}

	/* set op3 for remaining instructions */
	switch(opcode) {
		case LDSB:
			SET_OP3(*bin_instruction, OP3_LDSB);
			break;
		case LDSH:
			SET_OP3(*bin_instruction, OP3_LDSH);
			break;
		case LDUB:
			SET_OP3(*bin_instruction, OP3_LDUB);
			break;
		case LDUH:
			SET_OP3(*bin_instruction, OP3_LDUH);
			break;
		case LD:
			SET_OP3(*bin_instruction, OP3_LD);
			break;
		case LDD:
			SET_OP3(*bin_instruction, OP3_LDD);
			break;
		case LDSBA:
			SET_OP3(*bin_instruction, OP3_LDSBA);
			break;
		case LDSHA:
			SET_OP3(*bin_instruction, OP3_LDSHA);
			break;
		case LDUBA:
			SET_OP3(*bin_instruction, OP3_LDUBA);
			break;
		case LDUHA:
			SET_OP3(*bin_instruction, OP3_LDUHA);
			break;
		case LDA:
			SET_OP3(*bin_instruction, OP3_LDA);
			break;
		case LDDA:
			SET_OP3(*bin_instruction, OP3_LDDA);
			break;
		case STB:
			SET_OP3(*bin_instruction, OP3_STB);
			break;
		case STH:
			SET_OP3(*bin_instruction, OP3_STH);
			break;
		case ST:
			SET_OP3(*bin_instruction, OP3_ST);
			break;
		case STD:
			SET_OP3(*bin_instruction, OP3_STD);
			break;
		case STBA:
			SET_OP3(*bin_instruction, OP3_STBA);
			break;
		case STHA:
			SET_OP3(*bin_instruction, OP3_STHA);
			break;
		case STA:
			SET_OP3(*bin_instruction, OP3_STA);
			break;
		case STDA:
			SET_OP3(*bin_instruction, OP3_STDA);
			break;
		case LDSTUB:
			SET_OP3(*bin_instruction, OP3_LDSTUB);
			break;
		case LDSTUBA:
			SET_OP3(*bin_instruction, OP3_LDSTUBA);
			break;
		case SWAP:
			SET_OP3(*bin_instruction, OP3_SWAP);
			break;
		case SWAPA:
			SET_OP3(*bin_instruction, OP3_SWAPA);
			break;
		case AND:
			SET_OP3(*bin_instruction, OP3_AND);
			break;
		case ANDCC:
			SET_OP3(*bin_instruction, OP3_ANDCC);
			break;
		case ANDN:
			SET_OP3(*bin_instruction, OP3_ANDN);
			break;
		case ANDNCC:
			SET_OP3(*bin_instruction, OP3_ANDNCC);
			break;
		case OR:
			SET_OP3(*bin_instruction, OP3_OR);
			break;
		case ORCC:
			SET_OP3(*bin_instruction, OP3_ORCC);
			break;
		case ORN:
			SET_OP3(*bin_instruction, OP3_ORN);
			break;
		case ORNCC:
			SET_OP3(*bin_instruction, OP3_ORNCC);
			break;
		case XOR:
			SET_OP3(*bin_instruction, OP3_XOR);
			break;
		case XORCC:
			SET_OP3(*bin_instruction, OP3_XORCC);
			break;
		case XNOR:
			SET_OP3(*bin_instruction, OP3_XNOR);
			break;
		case XNORCC:
			SET_OP3(*bin_instruction, OP3_XNORCC);
			break;
		case SLL:
			SET_OP3(*bin_instruction, OP3_SLL);
			break;
		case SRL:
			SET_OP3(*bin_instruction, OP3_SRL);
			break;
		case SRA:
			SET_OP3(*bin_instruction, OP3_SRA);
			break;
		case ADD:
			SET_OP3(*bin_instruction, OP3_ADD);
			break;
		case ADDCC:
			SET_OP3(*bin_instruction, OP3_ADDCC);
			break;
		case ADDX:
			SET_OP3(*bin_instruction, OP3_ADDX);
			break;
		case ADDXCC:
			SET_OP3(*bin_instruction, OP3_ADDXCC);
			break;
		case TADDCC:
			SET_OP3(*bin_instruction, OP3_TADDCC);
			break;
		case TADDCCTV:
			SET_OP3(*bin_instruction, OP3_TADDCCTV);
			break;
		case SUB:
			SET_OP3(*bin_instruction, OP3_SUB);
			break;
		case SUBCC:
			SET_OP3(*bin_instruction, OP3_SUBCC);
			break;
		case SUBX:
			SET_OP3(*bin_instruction, OP3_SUBX);
			break;
		case SUBXCC:
			SET_OP3(*bin_instruction, OP3_SUBXCC);
			break;
		case TSUBCC:
			SET_OP3(*bin_instruction, OP3_TSUBCC);
			break;
		case TSUBCCTV:
			SET_OP3(*bin_instruction, OP3_TSUBCCTV);
			break;
		case MULSCC:
			SET_OP3(*bin_instruction, OP3_MULSCC);
			break;
		case UMUL:
			SET_OP3(*bin_instruction, OP3_UMUL);
			break;
		case SMUL:
			SET_OP3(*bin_instruction, OP3_SMUL);
			break;
		case UMULCC:
			SET_OP3(*bin_instruction, OP3_UMULCC);
			break;
		case SMULCC:
			SET_OP3(*bin_instruction, OP3_SMULCC);
			break;
		case UDIV:
			SET_OP3(*bin_instruction, OP3_UDIV);
			break;
		case SDIV:
			SET_OP3(*bin_instruction, OP3_SDIV);
			break;
		case UDIVCC:
			SET_OP3(*bin_instruction, OP3_UDIVCC);
			break;
		case SDIVCC:
			SET_OP3(*bin_instruction, OP3_SDIVCC);
			break;
		case SAVE:
			SET_OP3(*bin_instruction, OP3_SAVE);
			break;
		case RESTORE:
			SET_OP3(*bin_instruction, OP3_RESTORE);
			break;
		case JUMPL:
			SET_OP3(*bin_instruction, OP3_JUMPL);
			break;
		case RD:
			SET_OP3(*bin_instruction, OP3_RDY);
			break;
		case WR:
			SET_OP3(*bin_instruction, OP3_WRY);
			break;
		default: 
			fprintf(stderr, "Warning: not supported opcode encoutered!\n");
			*bin_instruction = 0;
			break;
	}

}

/**
  * @brief 
  * @param[in] outstream The filestream where to write the binary
  * instructions.
  */
void printInstructions(FILE* outstream) {

	sparc_instruction_node_t* instr_iter;
	sparc_instruction* cur_instruction;
	
	uint32_t instr_no, file_length = 0;

	uint32_t bin_instruction;
	
	int opcode;

	int32_t address_offset;
	int icc;
	int dst_reg, src1_reg, src2_reg;
	int immediate;

	int i;	

	/* get first instruction */
	instr_iter = gen_assembler->getFirstInstruction();

	/* iterate over all instructions */
	while(instr_iter) {

		/* reset bin_instruction */
		bin_instruction = 0;
		cur_instruction = &(instr_iter->instruction);

		/* get opcode */
		opcode = cur_instruction->opcode;

		/* get instruction number */
		instr_no = cur_instruction->instr_no;
		
		/* set instruction to correct format and opcode */
		init_bin_instruction(&bin_instruction, opcode);

		switch(opcode) {
			case CALL:
				address_offset = cur_instruction->operands[0].value.labeladdress;
				address_offset = address_offset - instr_no;
				/* set correct address for call instruction 
				   which is relative to current instruction */
				SET_DISP30(bin_instruction, address_offset);
				break;
			case BRANCH:
				address_offset = cur_instruction->operands[0].value.labeladdress;
				address_offset = address_offset - instr_no;
				icc = cur_instruction->operands[1].value.icc;
				/* set branch target address */
				SET_IMM22(bin_instruction, address_offset);
				/* set condition code */
				SET_CC(bin_instruction, icc);
				/* set A bit to zero for all branch instructions */
				SET_A(bin_instruction, 0);
				break;
			case NOP:
				/* nop is a special case of sethi */
				SET_RD(bin_instruction, 0);
				SET_IMM22(bin_instruction, 0);
				break;
			case SETHI:
				dst_reg = cur_instruction->operands[0].value.reg;
				immediate = cur_instruction->operands[1].value.imm22;
				/* set destination register */
				SET_RD(bin_instruction, dst_reg);
				/* set 22 bit immediate value */
				SET_IMM22(bin_instruction, immediate);
				break;
			/* nothing left to do for clearcycles instruction */
			case CYCLE_CLEAR:
			/* nothing left to do for printcycles instruction */
			case CYCLE_PRINT:
				break;
			/* handle hwloop start instruction */
			case HWLOOP_START:
				SET_HWLOOP_TYPE(bin_instruction, HWLOOP_TYPE_START);	
				break;
			/* handle hwloop init instructions */
			case HWLOOP_INIT:
				dst_reg = cur_instruction->operands[0].value.loopreg;
				/* address offset calculation for loopstart and end */
				address_offset = cur_instruction->operands[1].value.labeladdress;
				address_offset = address_offset - instr_no;
				/* init start register */
				if (dst_reg == LOOPS_REGISTER) {
					SET_HWLOOP_TYPE(bin_instruction, HWLOOP_TYPE_SET_S);
					SET_IMM22(bin_instruction, address_offset);
				/* init end register */
				} else if (dst_reg == LOOPE_REGISTER) {
					SET_HWLOOP_TYPE(bin_instruction, HWLOOP_TYPE_SET_E);
					SET_IMM22(bin_instruction, address_offset);
				/* init loop bound with other register */
				} else if (cur_instruction->operands[1].type == OPERAND_TYPE_REGISTER) {
					SET_HWLOOP_TYPE(bin_instruction, HWLOOP_TYPE_SET_B_REG);
					src1_reg = cur_instruction->operands[1].value.reg;
					SET_RS1(bin_instruction, src1_reg);
				/* init loop bound with immediate */
				} else if (cur_instruction->operands[1].type == OPERAND_TYPE_IMM22) {
					SET_HWLOOP_TYPE(bin_instruction, HWLOOP_TYPE_SET_B_IMM);
					immediate = cur_instruction->operands[1].value.imm22;
					SET_IMM22(bin_instruction, immediate);
				}
				break;
			case SEL:
				dst_reg = cur_instruction->operands[0].value.reg;
				SET_RD(bin_instruction, dst_reg);
				icc = cur_instruction->operands[3].value.icc;
				SELCC_SET_ICC(bin_instruction, icc);
				/* if source1 is a register, there may be two possibilities */
				if (cur_instruction->operands[1].type == OPERAND_TYPE_REGISTER) {
					src1_reg = cur_instruction->operands[1].value.reg;
					SELCC_SET_RS1(bin_instruction, src1_reg);
					/* if source2 is an immediate */
					if (cur_instruction->operands[2].type == OPERAND_TYPE_SIMM11) {
						SELCC_SET_TYPE(bin_instruction, SELCC_TYPE_REG_IMM);
						immediate = cur_instruction->operands[2].value.simm11;
						SELCC_SET_SIMM11(bin_instruction, immediate);
					} else {
					/* otherwise, source2 may only be a register */
						SELCC_SET_TYPE(bin_instruction, SELCC_TYPE_REG_REG);
						src2_reg = cur_instruction->operands[2].value.reg;
						SELCC_SET_RS2(bin_instruction, src2_reg);
					}
				/* if source2 is an immediate, there is only one remaining possibility */
				} else {
					SELCC_SET_TYPE(bin_instruction, SELCC_TYPE_IMM_IMM);
					immediate = cur_instruction->operands[1].value.simm8;
					SELCC_SET_SRC1_IMM8(bin_instruction, immediate);
					immediate = cur_instruction->operands[2].value.simm8;
					SELCC_SET_SRC2_IMM8(bin_instruction, immediate);
				}
				break;
			/* nearly everything has been done for predend instruction */
			case PREDEND:
				PRED_BLOCK_SET_TYPE(bin_instruction, PRED_BLOCK_TYPE_END);
				break;
			/* handle predbegin instruction */
			case PREDBEGIN:
				/* set correct type */
				PRED_BLOCK_SET_TYPE(bin_instruction, PRED_BLOCK_TYPE_BEGIN);
				/* set source operand */
				src2_reg = cur_instruction->operands[0].value.preg;
				SET_RS2(bin_instruction, src2_reg);
				/* set t/f bit */
				immediate = cur_instruction->operands[1].value.tf;
				PRED_BLOCK_SET_TF(bin_instruction, immediate);
				break; 
			case PREDCLEAR:
				/* predclear is only a special case of predset, using ICC "never" */
				PRED_BLOCK_SET_TYPE(bin_instruction, PRED_BLOCK_TYPE_CLEAR);
				PRED_BLOCK_SET_ICC(bin_instruction, CC_N);
				/* we only need destination register, no source or t/f flag */
				dst_reg = cur_instruction->operands[0].value.preg;
				SET_RD(bin_instruction, dst_reg);
				break;
			case PREDSET:
				PRED_BLOCK_SET_TYPE(bin_instruction, PRED_BLOCK_TYPE_SET);
				/* we only need destination register, no source or t/f flag */
				dst_reg = cur_instruction->operands[0].value.preg;
				SET_RD(bin_instruction, dst_reg);
				/* in case there is no icc, just use the "always" condition */
				if (cur_instruction->num_operands == 1) {
					PRED_BLOCK_SET_ICC(bin_instruction, CC_A);
				} else {
					icc = cur_instruction->operands[1].value.icc;
					PRED_BLOCK_SET_ICC(bin_instruction, icc);
				}
				break;
			/* read Y is the only remaining instruction with special treatment */
			case RD:
				dst_reg = cur_instruction->operands[0].value.reg;
				src1_reg = cur_instruction->operands[1].value.reg;
				if (src1_reg != Y_REGISTER_NO) {
					fprintf(stderr, "Warning: unknown source register for RD operation!\n");
				}
				/* set destination register */
				SET_RD(bin_instruction, dst_reg);
				/* source register should be Y register */
				SET_RS1(bin_instruction, src1_reg);
				break;
			/* all remaining operations have 3 operands */
			default:
				/* set destination register for current operation */
				dst_reg = cur_instruction->operands[0].value.reg;
				SET_RD(bin_instruction, dst_reg);
				/* set first source register for current operation */
				src1_reg = cur_instruction->operands[1].value.reg;
				SET_RS1(bin_instruction, src1_reg);

				/* source 2 may be either a register or an immediate */
				if (cur_instruction->operands[2].type == OPERAND_TYPE_REGISTER) {
					src2_reg = cur_instruction->operands[2].value.reg;
					SET_RS2(bin_instruction, src2_reg);
				} else if (cur_instruction->operands[2].type == OPERAND_TYPE_SIMM13) {
					immediate = cur_instruction->operands[2].value.simm13;
					SET_SIMM13(bin_instruction, immediate);
				} else {
					fprintf(stderr, "Warning: unknown operand type for instruction number %d!\n", 
						instr_no);
				}
				break;
		}

		/* write current instruction out to filestream in big endian format */
		for(i = 3; i >= 0; i--) {
			if (fputc(((bin_instruction >> (i*8)) & 0xff), outstream) == EOF) {
				fprintf(stderr, "Could not write to file!\n");
				return;
			}
		}
		
		/* each instruction has 4 bytes */
		file_length += 4;
		/* get next instruction */
		instr_iter = instr_iter->next_instruction;
	}

	/* go to begin of file and write target id */
	rewind(outstream);

	/* first two bytes are the target ID in big endian format */
	if (fputc(((TARGET_ID >> 8) & 0xff), outstream) == EOF) {
		fprintf(stderr, "Could not write to file!\n");
		return;
	}
	if (fputc((TARGET_ID & 0xff), outstream) == EOF) {
		fprintf(stderr, "Could not write to file!\n");
		return;
	}

	/* set file pointer to byte 7 which is the file length field */
	if (fseek(outstream, 6L, SEEK_SET)) {
		fprintf(stderr, "Could not set file position pointer to instruction length header!\n");
		return;
	}
	/* write out to file in big endian format */
	for (i = 3; i >= 0; i--) {
		if (fputc(((file_length >> (i*8)) & 0xff), outstream) == EOF) {
			fprintf(stderr, "Could not write to file!\n");
			return;
		}
	}


}

/**
  * @brief Returns 1 if the target supports conditional moves.
  */
int hasMovCC() {
	return 0;
}

/**
  * @brief Returns 1 if the target supports conditional selects.
  */
int hasSelCC() {
	return 1;
}

/**
  * @brief Returns 1 if the target supports hardware loops.
  */
int hasHWLoops() {
	return 1;
}

/**
  * @brief Returns 1 if the target supports predicated blocks
  * on integer condition codes.
  */
int hasPredBlocksCC() {
	return 0;
}

/**
  * @brief Returns 1 if the target supports predicated blocks
  * on predicate registers.
  */
int hasPredBlocksReg() {
	return 1;
}

/**
  * @brief Returns 1 if the target supports predicated instructions
  * on integer condition codes.
  */
int hasPredInstrsCC() {
	return 0;
}

/**
  * @brief Returns 1 if the target supports preidcated instructions
  * on predicate registers.
  */
int hasPredInstrsReg() {
	return 0;
}

/**
  * @brief Sets all needed target specific functions of assembler
  * object.
  * @param[in,out] assembler Pointer to assembler object which contains
  * all needed functions.
  * @return 0 on success, 1 otherwise.
  */
int assembler_init(gen_assembler_t* assembler) {

	assembler->hasMovCC = hasMovCC;
	assembler->hasSelCC = hasSelCC;
	assembler->hasHWLoops = hasHWLoops;
	assembler->hasPredBlocksCC = hasPredBlocksCC;
	assembler->hasPredBlocksReg = hasPredBlocksReg;
	assembler->hasPredInstrsCC = hasPredInstrsCC;
	assembler->hasPredInstrsReg = hasPredInstrsReg;
	assembler->printInstructions = printInstructions;

	gen_assembler = assembler;
	
	return 0;
}


