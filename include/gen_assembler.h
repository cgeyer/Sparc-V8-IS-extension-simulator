/*
 * SPARC V8 Instruction Set Extension Simulator
 *
 * File: include/gen_assembler.h
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

#ifndef __GEN_ASSEMBLER_H__
#define __GEN_ASSEMBLER_H__

#include <stdio.h>
#include <stdint.h>
#include "sparc_target.h"

#define SECTION_UNDEF	(-1)
#define	SECTION_TEXT	0
#define SECTION_DATA	1

typedef int (* check_attribute_fct_t)(void);
typedef void (* void_fct_t)(void);
typedef void (* label_fct_t)(unsigned, char*);
typedef void (* save_data_fct_t)(unsigned, int, unsigned); 
typedef void (* save_data_l_fct_t)(unsigned, char*, unsigned); 
typedef void (* save_branch_instr_fct_t)(unsigned, int, int, char*);
typedef void (* save_call_instr_fct_t)(unsigned, int, char*);
typedef void (* save_3op_instr_fct_t)(unsigned, int, int, int, int);
typedef void (* save_3op_l_instr_fct_t)(unsigned, int, int, int, char*);
typedef void (* save_2op_instr_fct_t)(unsigned, int, int, int);
typedef void (* save_1op_instr_fct_t)(unsigned, int, int);
typedef void (* save_0op_instr_fct_t)(unsigned, int);
typedef void (* save_addr_instr_fct_t)(unsigned, int, int, sparc_address*);
typedef void (* save_hwloop_init_fct_t)(unsigned, int, int, char*);
typedef void (* save_movcc_instr_fct_t)(unsigned, int, int, int, int);
typedef void (* save_selcc_instr_fct_t)(unsigned, int, int, int, int, int);
typedef void (* add_icc_pred_fct_t)(unsigned, int);
typedef void (* add_reg_pred_fct_t)(unsigned, int, int);
typedef sparc_address* (* sparc_address_fct_t)(int, sparc_operand_type, void*, sparc_operand_type);
typedef sparc_instruction_node_t* (* get_instr_fct_t)(void);
typedef void (* print_fct_t)(FILE*);

typedef struct {

	check_attribute_fct_t 	hasMovCC;
	check_attribute_fct_t 	hasSelCC;
	check_attribute_fct_t 	hasHWLoops;
	check_attribute_fct_t 	hasPredBlocksCC;
	check_attribute_fct_t 	hasPredBlocksReg;
	check_attribute_fct_t 	hasPredInstrsCC;
	check_attribute_fct_t 	hasPredInstrsReg;

	sparc_address_fct_t		saveAddress;
	save_data_fct_t			saveData;
	save_data_l_fct_t		saveDataLabel;
	label_fct_t				saveLabel;

	save_branch_instr_fct_t	saveBranchInstr;
	save_call_instr_fct_t 	saveCallInstr;
	save_3op_instr_fct_t	saveRegRegInstr;
	save_3op_instr_fct_t	saveRegImmInstr;
	save_3op_l_instr_fct_t	saveRegLabelInstr;
	save_2op_instr_fct_t	saveSethiInstr;
	save_branch_instr_fct_t	saveSethiLabelInstr;
	save_addr_instr_fct_t	saveAddrInstr;
	save_2op_instr_fct_t	saveRdInstr;

	save_movcc_instr_fct_t	saveMovCCInstr;
	save_selcc_instr_fct_t	saveSelCCRegRegInstr;
	save_selcc_instr_fct_t	saveSelCCRegImmInstr;
	save_selcc_instr_fct_t	saveSelCCImmImmInstr;
	save_0op_instr_fct_t	saveHWLoopStartInstr;
	save_hwloop_init_fct_t	saveHWLoopInitInstr;
	save_2op_instr_fct_t	saveHWLoopBoundRegInstr;
	save_2op_instr_fct_t	saveHWLoopBoundImmInstr;
	save_0op_instr_fct_t	savePredBeginInstr;
	save_1op_instr_fct_t	savePredRegInstr;
	save_0op_instr_fct_t	savePredendInstr;
	save_0op_instr_fct_t	saveSimulatorInstr;

	add_icc_pred_fct_t		addICC;
	add_reg_pred_fct_t		addPReg;

	void_fct_t				checkLabels;

	print_fct_t				printInstructions;
	print_fct_t				printData;
	get_instr_fct_t			getFirstInstruction;
	
	void_fct_t				cleanUp;

} gen_assembler_t;

typedef int (* assembler_init_fct_t)(gen_assembler_t* assembler);

int gen_assembler_init(gen_assembler_t* assembler);

#endif /* __GEN_ASSEMBLER_H__ */
