/*
 * SPARC V8 Instruction Set Extension Simulator
 *
 * File: src/gen_asm.c
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

#include "gen_assembler.h"
#include "sparc_target.h"
#include "sparc.tab.h"

/** Pointer to first label node of linked list. */
static label_node_t* first_label = 0;
/** Pointer to first instruction node of linked list. */
static sparc_instruction_node_t* instr_begin = 0;
/** Pointer to last instruction node of linked list. */
static sparc_instruction_node_t* instr_end = 0; 
/** Pointer to first data node of linked list. */
static sparc_data_node_t* data_begin = 0;
/** Pointer to last data node of linked list. */
static sparc_data_node_t* data_end = 0;

/** Declaration of externally defined error function. */
extern int yyerror(char* e);

/**
  * @brief Frees all allocated memory. Note that string addresses 
  *        are freed separately.
  */
void cleanUp(void) {

	label_node_t* label_iter = first_label;
	label_node_t* tmp_label;

	/* free label list */
	while (label_iter) {
		tmp_label = label_iter;
		label_iter = tmp_label->next_label;
		free(tmp_label);
	}
	first_label = 0;

	/* free instruction list */
	while (instr_begin) {
		instr_end = instr_begin;
		instr_begin = instr_end->next_instruction;
		free(instr_end->instruction.operands);
		free(instr_end);
	}

	instr_begin = 0;
	instr_end = 0;

	/* free data list */
	while (data_begin) {
		data_end = data_begin;
		data_begin = data_end->next_data;
		free(data_end);
	}

	data_begin = 0;
	data_end = 0;

}

/**
  * @brief Allocates memory for the new instruction and saves it
  *        at the end of the instruction linked list.
  * @param[in] opcode       The opcode of the current instruction as defined by
  *                         the yacc header file.
  * @param[in] instr_no     The instruction number of the current instruction. 
  *                         Needed for branch identification. 
  * @param[in] num_operands Holds the number of operands of the current instruction.
  * @param[in] operands     Array containing all operands of the current
  *                         instruction. The array size has to be specified by 
  *                         num_operands.
  */
static void saveInstruction(int 			opcode,
							unsigned 		instr_no,
							unsigned 		num_operands,
							sparc_operand* 	operands) {
	sparc_instruction_node_t* new_instr_node = malloc(sizeof(sparc_instruction_node_t)); 
	if (!new_instr_node) {
		yyerror("Could not allocate data for sparc instruction!");
	}
	new_instr_node->instruction.opcode = opcode;
	new_instr_node->instruction.instr_no = instr_no;
	new_instr_node->instruction.num_operands = num_operands;
	new_instr_node->instruction.operands = operands;
	if (!instr_end) {
		instr_begin = new_instr_node;
		instr_end = new_instr_node;
	} else {
		instr_end->next_instruction = new_instr_node;
		instr_end = new_instr_node;
	}
}

/**
  * @brief Allocates a new address structure, consistung of two operands.
  *        Type and value of the operands are specified by the input 
  *        parameters.
  * @param[in] operand1      Holds the first operand of the current address.
  * @param[in] operand1_type May only be a register, but must be specified.
  * @param[in] operand2      Holds the second operand of the current address.
  *                          A void pointer is used to allow saving of
  *                          immediates, registers and labels.
  * @param[in] operand2_type May be either a register, a 13-bit signed
  *                          immediate or a label.
  * @return Pointer to the new allocated address data structure. It will
  *         be freed by function saveAddrInstr(). 
  */
sparc_address* saveAddress(	int 				operand1,
							sparc_operand_type 	operand1_type,
							void*				operand2,
							sparc_operand_type 	operand2_type) {

	sparc_address* new_sparc_address = 0;
	
	new_sparc_address = malloc(sizeof(sparc_address));

	/* fprintf(stderr, "Saving new address...\n"); */
	
	if (!new_sparc_address) {
		yyerror("Could not allocate memory for sparc address!"); 
	}
	
	new_sparc_address->operand1.type = operand1_type;
	new_sparc_address->operand1.value.reg = operand1;

	new_sparc_address->operand2.type = operand2_type;
	if (operand2_type == OPERAND_TYPE_REGISTER) {
		new_sparc_address->operand2.value.reg = (int) operand2;
	} else if (operand2_type == OPERAND_TYPE_SIMM13) {
		if (IS_SIMM13((int) operand2)) {
			new_sparc_address->operand2.value.simm13 = (int) operand2;
		} else {
			yyerror("No valid signed 13-bit immediate!");
		}
	} else if (operand2_type == OPERAND_TYPE_LOW_LABEL) {
		new_sparc_address->operand2.value.label = (char *) operand2;
	}

	return new_sparc_address;

}

/**
  * @brief Saves the given data in a new data node object and appends it
  *        to the end of the data linked list.
  * @param[in] data_no  Data number which corresponds to the absolute
  *                     address of the given data.
  * @param[in] value    The value to save.
  * @param[in] no_bytes Size of the current data value. May be 1, 2 or 4.
  */
void saveData(unsigned data_no, int value, unsigned no_bytes) {

	sparc_data_node_t* new_data_node = malloc(sizeof(sparc_instruction_node_t)); 
	if (!new_data_node) {
		yyerror("Could not allocate memory for data!");
	}
	new_data_node->value = (uint32_t) value;
	/* current node is not associated with any label */
	new_data_node->label = (char *) 0;
	new_data_node->no_bytes = no_bytes;
	new_data_node->data_no = data_no;
	if (!data_end) {
		data_begin = new_data_node;
		data_end = new_data_node;
	} else {
		data_end->next_data = new_data_node;
		data_end = new_data_node;
	}
	
	/* fprintf(stderr, "Saving data %d at address number %d.\n", value, data_no); */
}

/**
  * @brief Saves the given pointer to a label in a new data node 
  *        object and appends it to the end of the data linked list.
  * @param[in] data_no  Data number which corresponds to the absolute
  *                     address of the given data.
  * @param[in] label    The label associated with the data or instruction
  *                     address to be saved.
  * @param[in] no_bytes Size of the current data value. Must always be 4.
  */
void saveDataLabel(unsigned data_no, char* label, unsigned no_bytes) {

	sparc_data_node_t* new_data_node = malloc(sizeof(sparc_instruction_node_t)); 
	if (!new_data_node) {
		yyerror("Could not allocate memory for data!");
	}
	new_data_node->value = (uint32_t) 0;
	/* current node is not associated with any label */
	new_data_node->label = label;
	new_data_node->no_bytes = no_bytes;
	new_data_node->data_no = data_no;
	if (!data_end) {
		data_begin = new_data_node;
		data_end = new_data_node;
	} else {
		data_end->next_data = new_data_node;
		data_end = new_data_node;
	}
	
	/* fprintf(stderr, "Saving data pointer %s at address number %d.\n", label, data_no); */
}

/**
  * @brief Saves a new label (data or instruction) in a new node
  *        at the beginning of the label linked list.
  * @param[in] address    Associated address of the label.
  * @param[in] label_name The name of the label which will be used
  *                       as key for refinding it. Must be unique
  *                       within a single file.
  */
void saveLabel(unsigned address, char* label_name) {
	label_node_t* new_label;
	label_node_t* label_iter = first_label;

	/* check whether label has not been saved yet */
	while (label_iter) {
		if (!(strcmp(label_iter->label_name, label_name))) {
			yyerror("Label already exists but must be unique!");
		}
		label_iter = label_iter->next_label;
	}

	/* create new label */
	new_label = malloc(sizeof(label_node_t));
	if (!new_label) {
		yyerror("Could not allocate memory for label!"); 
	}

	/* append label at begin of list */
	new_label->label_name = label_name;
	new_label->address = address;
	new_label->next_label = first_label;
	first_label = new_label; 

	/* fprintf(stderr, "Saving label \"%s\" at address number %d.\n", label_name, address); */
}

/**
  * @brief Returns the address of the given label if it exists or -1.
  * @param[in] label The name of the label to find.
  * @return The address of the given label or -1 casted to
  *         unsigned (which is 0xffffffff for 32 bit) in the case the
  *         label could not be found.
  */
static unsigned getLabelAddress(const char* label) {

	label_node_t* label_iter = first_label;
	unsigned address = (unsigned) -1;

	while(label_iter) {
		if (!strcmp(label, label_iter->label_name)) {
			address = label_iter->address;
			break;
		}
		label_iter = label_iter->next_label;
	}
	return address;
}

/**
  * @brief Saves a branch instruction with the given parameters.
  * @param[in] instr_no   The number of the current instruction.
  * @param[in] opcode     The opcode of the current instruction.
  * @param[in] icc        The integer condition code when a branch
  *                       shall be taken.
  * @param[in] label_name The label name of the branch target.
  */
void saveBranchInstr(unsigned instr_no, int opcode, int icc, char* label_name) {

	sparc_operand* operands = malloc(sizeof(sparc_operand)*2); 
	if (!operands) {
		yyerror("Could not allocate memory for operands!");
	}
	operands[0].type = OPERAND_TYPE_LABEL;
	operands[0].value.label = label_name;

	operands[1].type = OPERAND_TYPE_ICC;
	operands[1].value.icc = icc;

	saveInstruction(opcode, instr_no, 2, operands);

	/* fprintf(stderr, "Saved Branch instruction (%d)!\n", instr_no); */

}

/**
  * @brief Saves a call instruction with the given parameters.
  * @param[in] instr_no	  The number of the current instruction.
  * @param[in] opcode     The opcode of the current instruction.
  * @param[in] label_name The label name of the function which will 
  *                       be called.
  */
void saveCallInstr(unsigned instr_no, int opcode, char* label_name) {

	sparc_operand* operands = malloc(sizeof(sparc_operand)); 
	if (!operands) {
		yyerror("Could not allocate memory for operands!");
	}
	operands[0].type = OPERAND_TYPE_LABEL;
	operands[0].value.label = label_name;

	saveInstruction(opcode, instr_no, 1, operands);

	/* fprintf(stderr, "Saved Call instruction (%d)!\n", instr_no); */


}

/**
  * @brief Saves an instruction with two source and one 
  *        destination register.
  * @param[in] instr_no The number of the current instruction.
  * @param[in] opcode   The opcode of the current instruction.
  * @param[in] dest_reg Number of the destination register.
  * @param[in] src_reg1 Number of the first source register.
  * @param[in] src_reg2 Number of the second source register.
  */
void saveRegRegInstr(unsigned instr_no, int opcode, int dest_reg,
					 int src_reg1, int src_reg2) {

	sparc_operand* operands = malloc(sizeof(sparc_operand)*3); 
	if (!operands) {
		yyerror("Could not allocate memory for operands!");
	}

	operands[0].type = OPERAND_TYPE_REGISTER;
	operands[0].value.reg = dest_reg;

	operands[1].type = OPERAND_TYPE_REGISTER;
	operands[1].value.reg = src_reg1;

	operands[2].type = OPERAND_TYPE_REGISTER;
	operands[2].value.reg = src_reg2;

	saveInstruction(opcode, instr_no, 3, operands);

	/* fprintf(stderr, "Saved RegReg instruction (%d)!\n", instr_no); */

}

/**
  * @brief Saves an instruction with one source register, one
  *        immediate source and one destination register.
  * @param[in] instr_no The number of the current instruction.
  * @param[in] opcode   The opcode of the current instruction.
  * @param[in] dest_reg Number of the destination register.
  * @param[in] src_reg1 Number of the first source register.
  * @param[in] src_imm2 13-bit signed immediate source2 operand.
  */
void saveRegImmInstr(unsigned instr_no, int opcode, int dest_reg,
					 int src_reg1, int src_imm2) {
	sparc_operand* operands;

	if (!IS_SIMM13(src_imm2)) {
		yyerror("No valid signed 13-bit immediate!");
	}

	operands = malloc(sizeof(sparc_operand)*3); 
	if (!operands) {
		yyerror("Could not allocate memory for operands!");
	}

	operands[0].type = OPERAND_TYPE_REGISTER;
	operands[0].value.reg = dest_reg;

	operands[1].type = OPERAND_TYPE_REGISTER;
	operands[1].value.reg = src_reg1;

	operands[2].type = OPERAND_TYPE_SIMM13;
	operands[2].value.reg = src_imm2;

	saveInstruction(opcode, instr_no, 3, operands);

	/* fprintf(stderr, "Saved RegImm instruction (%d)!\n", instr_no); */

}

/**
  * @brief Saves an instruction with one source register, one immediate
  *        source and one destination register. 
  * @details The immediate will be determined by the lower 10 bits of 
  *          the address of the given label name.
  * @param[in] instr_no The number of the current instruction.
  * @param[in] opcode   The opcode of the current instruction.
  * @param[in] dest_reg Number of the destination register.
  * @param[in] src_reg1 Number of the first source register.
  * @param[in] label    Label of instruction or data section.
  */
void saveRegLabelInstr(unsigned instr_no, int opcode, int dest_reg,
					 int src_reg1, char* label) {

	sparc_operand* operands = malloc(sizeof(sparc_operand)*3); 

	if (!operands) {
		yyerror("Could not allocate memory for operands!");
	}

	operands[0].type = OPERAND_TYPE_REGISTER;
	operands[0].value.reg = dest_reg;

	operands[1].type = OPERAND_TYPE_REGISTER;
	operands[1].value.reg = src_reg1;

	operands[2].type = OPERAND_TYPE_LOW_LABEL;
	operands[2].value.label = label;

	saveInstruction(opcode, instr_no, 3, operands);

	/* fprintf(stderr, "Saved RegLabel instruction (%d)!\n", instr_no); */

}


/**
  * @brief Saves an address as source operands and one 
  *        destination register. 
  * @details The memory of the given address structure will be freed.
  * @param[in] instr_no The number of the current instruction.
  * @param[in] opcode   The opcode of the current instruction.
  * @param[in] dest_reg Number of the destination register.
  * @param[in] address  Address data object which was previously
  *                     created by the saveAddress() function.
  */
void saveAddrInstr(unsigned instr_no, int opcode, int dest_reg,
						sparc_address* address) {
	sparc_operand* operands = malloc(sizeof(sparc_operand)*3);
	if (!operands) {
		yyerror("Could not allocate memory for operands!");
	}

	operands[0].type = OPERAND_TYPE_REGISTER;
	operands[0].value.reg = dest_reg;

	operands[1].type = OPERAND_TYPE_REGISTER;
	operands[1].value.reg = address->operand1.value.reg;
	
	if (address->operand2.type == OPERAND_TYPE_REGISTER) {
		operands[2].type = OPERAND_TYPE_REGISTER;
		operands[2].value.reg = address->operand2.value.reg;
	} else if (address->operand2.type == OPERAND_TYPE_SIMM13) {
		operands[2].type = OPERAND_TYPE_SIMM13;
		operands[2].value.simm13 = address->operand2.value.simm13;
	} else if (address->operand2.type == OPERAND_TYPE_LOW_LABEL) {
		operands[2].type = OPERAND_TYPE_LOW_LABEL;
		operands[2].value.label = address->operand2.value.label;
	} else {
		free(operands);
		free(address);
		yyerror("Unknown type for second address operand!");
	}

	/* frees previously allocated memory */
	free(address);

	saveInstruction(opcode, instr_no, 3, operands);

	/* fprintf(stderr, "Saved Address instruction (%d)!\n", instr_no); */
	
}

/**
  * @brief Saves a sethu instruction with one 22-bit immediate
  *        source and one destination register.
  * @param[in] instr_no The number of the current instruction.
  * @param[in] opcode   The opcode of the current instruction.
  * @param[in] dest_reg Number of the destination register.
  * @param[in] imm22    The immediate representing the 22 most
  *                     significant bits of the value to be saved.
  */
void saveSethiInstr(unsigned instr_no, int opcode, int dest_reg,
					int imm22) {
	sparc_operand* operands;

	if (!IS_UIMM22(imm22)) {
		yyerror("No valid unsigned 22-bit immediate!");
	}

	operands = malloc(sizeof(sparc_operand)*2);

	if (!operands) {
		yyerror("Could not allocate memory for operands!");
	}
	
	operands[0].type = OPERAND_TYPE_REGISTER;
	operands[0].value.reg = dest_reg;

	operands[1].type = OPERAND_TYPE_IMM22;
	operands[1].value.imm22 = imm22;

	saveInstruction(opcode, instr_no, 2, operands);

	/* fprintf(stderr, "Saved Sethi instruction (%d)!\n", instr_no); */
}

/**
  * @brief Saves a sethi instruction with one 22-bit immediate
  *        source and one destination register.
  * @details The upper 22 bits of the address of the label will 
  *          be used as source operands.
  * @param[in] instr_no The number of the current instruction.
  * @param[in] opcode   The opcode of the current instruction.
  * @param[in] dest_reg Number of the destination register.
  * @param[in] label    Label of instruction or data section.
  */
void saveSethiLabelInstr(unsigned instr_no, int opcode, int dest_reg,
					char* label) {
	sparc_operand* operands;

	operands = malloc(sizeof(sparc_operand)*2);

	if (!operands) {
		yyerror("Could not allocate memory for operands!");
	}
	
	operands[0].type = OPERAND_TYPE_REGISTER;
	operands[0].value.reg = dest_reg;

	operands[1].type = OPERAND_TYPE_HI_LABEL;
	operands[1].value.label = label;

	saveInstruction(opcode, instr_no, 2, operands);

	/* fprintf(stderr, "Saved SethiLabel instruction (%d)!\n", instr_no); */
}


/**
  * @brief Saves a read instruction with one source and one 
  *        destination register.
  * @param[in] instr_no The number of the current instruction.
  * @param[in] opcode   The opcode of the current instruction.
  * @param[in] dest_reg Number of the destination register.
  * @param[in] src_reg  Number of the source register which
  *                     is always the number of the y-register.
  * @param[in] src_reg2 Number of the second source register.
  */
void saveRdInstr(unsigned instr_no, int opcode, int dest_reg,
				 int src_reg) {

	sparc_operand* operands = malloc(sizeof(sparc_operand)*2);
	
	if (!operands) {
		yyerror("Could not allocate memory for operands!");
	}

	operands[0].type = OPERAND_TYPE_REGISTER;
	operands[0].value.reg = dest_reg;

	operands[1].type = OPERAND_TYPE_REGISTER;
	operands[1].value.reg = src_reg;

	saveInstruction(opcode, instr_no, 2, operands);
	
	/* fprintf(stderr, "Saved Rd instruction (%d)!\n", instr_no); */

}

/**
  * @brief Saves a conditional move instruction with one source register, 
  *        one destination register and an integer condition code.
  * @param[in] instr_no The number of the current instruction.
  * @param[in] opcode   The opcode of the current instruction.
  * @param[in] dest_reg Number of the destination register.
  * @param[in] sel_reg  Number of the register to select.
  * @param[in] icc      The integer condition code of the conditional move.
  */
void saveMovCCInstr(unsigned instr_no, int opcode, int dest_reg,
					int sel_reg, int icc) {
	sparc_operand* operands = malloc(sizeof(sparc_operand)*3);
	
	if (!operands) {
		yyerror("Could not allocate memory for operands!");
	}

	operands[0].type = OPERAND_TYPE_REGISTER;
	operands[0].value.reg = dest_reg;

	operands[1].type = OPERAND_TYPE_REGISTER;
	operands[1].value.reg = sel_reg;

	operands[2].type = OPERAND_TYPE_ICC;
	operands[2].value.icc = icc;

	saveInstruction(opcode, instr_no, 3, operands);

	/* fprintf(stderr, "Saved MovCC instruction (%d)!\n", instr_no); */

}

/**
  * @brief Saves a conditional select instruction with two source 
  *        registers, one destination register and an integer 
  *        condition code.
  * @param[in] instr_no The number of the current instruction.
  * @param[in] opcode   The opcode of the current instruction.
  * @param[in] dest_reg Number of the destination register.
  * @param[in] sel_reg1 Number of the register to select on icc true.
  * @param[in] sel_reg2 Number of the register to select on icc false.
  * @param[in] icc      The integer condition code of the conditional select.
  */
void saveSelCCRegRegInstr(unsigned instr_no, int opcode, int dest_reg,
						int sel_reg1, int sel_reg2, int icc) {
	sparc_operand* operands = malloc(sizeof(sparc_operand)*4);
	
	if (!operands) {
		yyerror("Could not allocate memory for operands!");
	}

	operands[0].type = OPERAND_TYPE_REGISTER;
	operands[0].value.reg = dest_reg;

	operands[1].type = OPERAND_TYPE_REGISTER;
	operands[1].value.reg = sel_reg1;

	operands[2].type = OPERAND_TYPE_REGISTER;
	operands[2].value.reg = sel_reg2;

	operands[3].type = OPERAND_TYPE_ICC;
	operands[3].value.icc = icc;

	saveInstruction(opcode, instr_no, 4, operands);

	/* fprintf(stderr, "Saved SelCC instruction (%d)!\n", instr_no); */

}

/**
  * @brief Saves a conditional select instruction with one source 
  *        register, one 11 bit signed immediate source operand,
  *        one destination register and an integer condition code.
  * @param[in] instr_no  The number of the current instruction.
  * @param[in] opcode    The opcode of the current instruction.
  * @param[in] dest_reg  Number of the destination register.
  * @param[in] sel_reg1  Number of the register to select on icc true.
  * @param[in] sel_simm2 11-bit signed immediate source operand to 
  *                      select on icc false.
  * @param[in] icc       The integer condition code of the conditional select.
  */
void saveSelCCRegImmInstr(unsigned instr_no, int opcode, int dest_reg,
						int sel_reg1, int sel_simm2, int icc) {
	sparc_operand* operands;

	if (!IS_SIMM11(sel_simm2)) {
		yyerror("No valid 11-bit signed immediate!");
	}

	operands = malloc(sizeof(sparc_operand)*4);
	
	if (!operands) {
		yyerror("Could not allocate memory for operands!");
	}

	operands[0].type = OPERAND_TYPE_REGISTER;
	operands[0].value.reg = dest_reg;

	operands[1].type = OPERAND_TYPE_REGISTER;
	operands[1].value.reg = sel_reg1;

	operands[2].type = OPERAND_TYPE_SIMM11;
	operands[2].value.simm11 = sel_simm2;

	operands[3].type = OPERAND_TYPE_ICC;
	operands[3].value.icc = icc;

	saveInstruction(opcode, instr_no, 4, operands);

	/* fprintf(stderr, "Saved SelCC instruction (%d)!\n", instr_no); */

}

/**
  * @brief Saves a conditional select instruction with two 8-bit
  *        signed immediate source operands, one destination 
  *        register and an integer condition code.
  * @param[in] instr_no  The number of the current instruction.
  * @param[in] opcode    The opcode of the current instruction.
  * @param[in] dest_reg  Number of the destination register.
  * @param[in] sel_simm1 8-bit signed immediate source operand to 
  *                      select on icc true.
  * @param[in] sel_simm2 8-bit signed immediate source operand to 
  *                      select on icc false.
  * @param[in] icc       The integer condition code of the conditional select.
  */
void saveSelCCImmImmInstr(unsigned instr_no, int opcode, int dest_reg,
						int sel_simm1, int sel_simm2, int icc) {
	sparc_operand* operands;

	if (!IS_SIMM8(sel_simm1)) {
		yyerror("No valid 8-bit signed immediate!");
	}

	if (!IS_SIMM8(sel_simm2)) {
		yyerror("No valid 8-bit signed immediate!");
	}

	operands = malloc(sizeof(sparc_operand)*4);
	
	if (!operands) {
		yyerror("Could not allocate memory for operands!");
	}

	operands[0].type = OPERAND_TYPE_REGISTER;
	operands[0].value.reg = dest_reg;

	operands[1].type = OPERAND_TYPE_SIMM8;
	operands[1].value.simm8 = sel_simm1;

	operands[2].type = OPERAND_TYPE_SIMM8;
	operands[2].value.simm8 = sel_simm2;

	operands[3].type = OPERAND_TYPE_ICC;
	operands[3].value.icc = icc;

	saveInstruction(opcode, instr_no, 4, operands);

	/* fprintf(stderr, "Saved MovCC instruction (%d)!\n", instr_no); */

}

/**
  * @brief Saves an instruction without any operands.
  * @param[in] instr_no  The number of the current instruction.
  * @param[in] opcode    The opcode of the current instruction.
  */
void saveZeroOperandInstr(unsigned instr_no, int opcode) {
	saveInstruction(opcode, instr_no, 0, NULL);
}

/**
  * @brief Saves a hwloop init instruction with a destination
  *        register.
  * @param[in] instr_no The number of the current instruction.
  * @param[in] opcode   The opcode of the current instruction.
  * @param[in] reg      The destination register of the current
  *                     instruction.
  * @param[in] label    The label indicating the address of the 
  *                     first loop instruction or the first 
  *                     instruction following the loop.
  */
void saveHWLoopInitInstr(unsigned instr_no, int opcode, int reg, 
						char* label) {

	sparc_operand* operands = malloc(sizeof(sparc_operand)*2);

	if (!operands) {
		yyerror("Could not allocate memory for operands!");
	}

	operands[0].type = OPERAND_TYPE_LOOP_REG;
	operands[0].value.loopreg = reg;

	operands[1].type = OPERAND_TYPE_LABEL;
	operands[1].value.label = label;

	saveInstruction(opcode, instr_no, 2, operands);

	/* fprintf(stderr, "Saved HWLoopInit instruction (%d)!\n", instr_no); */

}

/**
  * @brief Saves a hwloop init loopbound instruction one source register.
  * @param[in] instr_no The number of the current instruction.
  * @param[in] opcode   The opcode of the current instruction.
  * @param[in] dest_reg The destination register of the current instruction.
  * @param[in] src_reg  The source register of the current instruction.
  */
void saveHWLoopBoundRegInstr(unsigned instr_no, int opcode, int dest_reg, 
							int src_reg) {

	sparc_operand* operands = malloc(sizeof(sparc_operand)*2);

	if (!operands) {
		yyerror("Could not allocate memory for operands!");
	}

	operands[0].type = OPERAND_TYPE_LOOP_REG;
	operands[0].value.loopreg = dest_reg;

	operands[1].type = OPERAND_TYPE_REGISTER;
	operands[1].value.reg = src_reg;

	saveInstruction(opcode, instr_no, 2, operands);

	/* fprintf(stderr, "Saved HWLoopInit instruction (%d)!\n", instr_no); */

}

/**
  * @brief Saves a hwloop init loopbound instruction one immediate source.
  * @param[in] instr_no The number of the current instruction.
  * @param[in] opcode   The opcode of the current instruction.
  * @param[in] dest_reg The destination register of the current instruction.
  * @param[in] src_imm  The 22-bit unsigned immediate source operand of 
  *                     the current instruction.
  */
void saveHWLoopBoundImmInstr(unsigned instr_no, int opcode, int dest_reg, 
							int src_imm) {

	sparc_operand* operands;

	if (!IS_IMM22(src_imm)) {
		yyerror("No valid 22-bit signed immediate!");
	}
	
	operands = malloc(sizeof(sparc_operand)*2);

	if (!operands) {
		yyerror("Could not allocate memory for operands!");
	}

	operands[0].type = OPERAND_TYPE_LOOP_REG;
	operands[0].value.loopreg = dest_reg;

	operands[1].type = OPERAND_TYPE_IMM22;
	operands[1].value.imm22 = src_imm;

	saveInstruction(opcode, instr_no, 2, operands);

	/* fprintf(stderr, "Saved HWLoopInit instruction (%d)!\n", instr_no); */

}

/**
  * @brief Saves an instruction with a predicate register as
  *        destination.
  * @param[in] instr_no The number of the current instruction.
  * @param[in] opcode   The opcode of the current instruction.
  * @param[in] preg     The destination register of the current instruction.
  */
void savePredRegInstr(unsigned instr_no, int opcode, int preg) {

	sparc_operand* operands = malloc(sizeof(sparc_operand));

	if (!operands) {
		yyerror("Could not allocate memory for operands!");
	}

	operands[0].type = OPERAND_TYPE_PREG;
	operands[0].value.preg = preg;

	saveInstruction(opcode, instr_no, 1, operands);

	/* fprintf(stderr, "Saved PredReg instruction (%d)!\n", instr_no); */

}

/**
  * @brief Adds an integer condition predicate to the current
  *        instruction.
  * @param[in] instr_no The instruction number of the instruction
  *                     which will be appended by the icc. Must be
  *                     equal with the instruction number of the
  *                     last saved instruction.
  * @param[in] icc      The integer condition code which will be
  *                     added to the given instruction.
  */
void addICC(unsigned instr_no, int icc) {

	unsigned num_operands; 
	sparc_operand* old_operands;
	sparc_operand* new_operands;

	/* instruction must exist and has to be equal to last created 
	   instruction */
	if ((instr_end) && (instr_end->instruction.instr_no == instr_no)) {

		num_operands = instr_end->instruction.num_operands;
		old_operands = instr_end->instruction.operands;

		/* allocate memory for one additional operand */
		new_operands = realloc(old_operands, sizeof(sparc_operand)*(num_operands + 1));

		if (!new_operands) {
			yyerror("Could not allocate memory for ICC predicate!");
		}

		/* save new ICC predicate at end of operand array */
		new_operands[num_operands].type = OPERAND_TYPE_ICC;
		new_operands[num_operands].value.icc = icc;

		/* set number of operands to new value */
		instr_end->instruction.num_operands = num_operands + 1;
		/* save newly allocated address for operands */
		instr_end->instruction.operands = new_operands;

		/* fprintf(stderr, "Added new ICC predicate for instruction no %d!\n", instr_no); */


	} else {
		yyerror("Unknown instruction number when trying to add ICC predicate!");
	}
}

/**
  * @brief Adds a predicate to the current instruction.
  * @param[in] instr_no The instruction number of the instruction
  *                     which will be appended by the predicate. 
  *                     Must be equal with the instruction number 
  *                     of the last saved instruction.
  * @param[in] preg     The register number of the predicate register.
  * @param[in] tf       The t/f-bit of the predicate register.
  */
void addPReg(unsigned instr_no, int preg, int tf) {

	unsigned num_operands; 
	sparc_operand* old_operands;
	sparc_operand* new_operands;

	/* instruction must exist and has to be equal to last created 
	   instruction */
	if ((instr_end) && (instr_end->instruction.instr_no == instr_no)) {

		num_operands = instr_end->instruction.num_operands;
		old_operands = instr_end->instruction.operands;

		/* allocate memory for one additional operand */
		new_operands = realloc(old_operands, sizeof(sparc_operand)*(num_operands + 2));

		if (!new_operands) {
			yyerror("Could not allocate memory for predicate register!");
		}

		/* save new predicate register */
		new_operands[num_operands].type = OPERAND_TYPE_PREG;
		new_operands[num_operands].value.preg = preg;

		/* save new predicate register info at end of operand array */
		new_operands[num_operands + 1].type = OPERAND_TYPE_TF;
		new_operands[num_operands + 1].value.tf = tf;

		/* set number of operands to new value */
		instr_end->instruction.num_operands = num_operands + 2;
		/* save newly allocated address for operands */
		instr_end->instruction.operands = new_operands;

		/* fprintf(stderr, "Added new register predicate for instruction no %d!\n", instr_no); */


	} else {
		yyerror("Unknown instruction number when trying to add ICC predicate!");
	}
}

/**
  * @brief Checks all instructions which have a label operand
  *        if the label exists and replaces the label with the
  *        corresponding absolute address.
  */
void checkLabels(void) {
	sparc_instruction_node_t* instr_it = instr_begin;
	sparc_operand* operands;
	sparc_data_node_t* data_iter = data_begin;
	unsigned num_operands, i, address, instr_counter;

	instr_counter = 0;

	/* check whole instruction list */
	while(instr_it) {
		if (instr_counter != instr_it->instruction.instr_no) {
			fprintf(stderr, "Warning: wrong instruction counter value!\n");
		}
		num_operands = instr_it->instruction.num_operands;
		operands = instr_it->instruction.operands;
		/* check for all operands whether they are a label */
		for (i = 0; i < num_operands; i++) {
			/* "normal" label as used e.g. for branch or call instructions */
			if (operands[i].type == OPERAND_TYPE_LABEL) {
				address = getLabelAddress(operands[i].value.label);
				if (address != (unsigned) -1) {
					free(operands[i].value.label);
					operands[i].type = OPERAND_TYPE_LABEL_ADDRESS;
					operands[i].value.labeladdress = address;
				} else {
					fprintf(stderr, "Unknown label \"%s\" for instruction number %d!\n",
						operands[i].value.label, instr_it->instruction.instr_no);
					cleanUp();
					exit(EXIT_FAILURE);
				}
			/* "hi" or "lo" label as used e.g. for arithmetic or logical instructions */
			} else if ((operands[i].type == OPERAND_TYPE_HI_LABEL) || 
				(operands[i].type == OPERAND_TYPE_LOW_LABEL)) {
				address = getLabelAddress(operands[i].value.label);
				if (address != (unsigned) -1) {
					free(operands[i].value.label);
					/* if we have a "hi" label, save upper 22 bits of address */
					if (operands[i].type == OPERAND_TYPE_HI_LABEL) {
						operands[i].type = OPERAND_TYPE_IMM22;
						operands[i].value.imm22 = ((address >> 10) & 0x3fffff);
						/* fprintf(stderr, "Found HI-label address %x!\n", operands[i].value.imm22); */
					/* if we have a "lo" label, save lower 10 bits of address */
					} else {
						operands[i].type = OPERAND_TYPE_SIMM13;
						operands[i].value.simm13 = (address & 0x3ff);
						/* fprintf(stderr, "Found LO-label address %x!\n", operands[i].value.simm13); */
					}
				} else {
					fprintf(stderr, "Unknown label \"%s\" for instruction number %d!\n",
						operands[i].value.label, instr_it->instruction.instr_no);
					cleanUp();
					exit(EXIT_FAILURE);

				}
			}
		}
		instr_it = instr_it->next_instruction;
		instr_counter++;
	}

	/* check whole data list */
	while (data_iter) {
		/* if the current data node is pointer to address */
		if (data_iter->label) {
			address = getLabelAddress(data_iter->label);
			if (address != (unsigned) -1) {
				data_iter->value = (uint32_t) address;
				data_iter->label = 0;
			} else {
				fprintf(stderr, "Unknown label \"%s\" for data address %d!\n",
					data_iter->label, data_iter->data_no);
				cleanUp();
				exit(EXIT_FAILURE);

			}
		}
		data_iter = data_iter->next_data;
	}
}

/**
  * @brief Get the pointer to the first saved instruction node.
  * @return Pointer to the first saved instruction node.
  */
sparc_instruction_node_t* getFirstInstruction(void) {
	return instr_begin;
}

/**
  * @brief Prints all data nodes as binary output to the given file
  *        stream. All data is saved in big endian format.
  * @param[in] outstream The file stream where to print the binary data.
  */
void printData(FILE* outstream) {

	uint32_t data_length = 0;
	uint32_t last_address = 0, cur_address = 0;
	uint32_t value;

	sparc_data_node_t* data_iter = data_begin;

	int i;
	unsigned no_bytes;
	int last_no_bytes = 0;

	/* perform 10 bytes dummy write */
	fprintf(outstream, "          ");

	/* print out data */
	while(data_iter) {
		cur_address = data_iter->data_no;
		no_bytes = data_iter->no_bytes;
		value = data_iter->value;
		/* handle skip instructions: fill memory with zeros */
		for (i = 0; i < (((int) (cur_address - last_address)) - last_no_bytes); i++) {
			if (fputc(0, outstream) == EOF) {
				fprintf(stderr, "Could not write to file!\n");
				return;
			}
			data_length++;
		}
		/* print out data */
		for (i = (int) no_bytes - 1; i >= 0; i--) {
			if (fputc(((value >> (i*8)) & 0xff), outstream) == EOF) {
				fprintf(stderr, "Could not write to file!\n");
				return;
			}
			data_length++;
		}

		last_address = cur_address;
		last_no_bytes = (int) no_bytes;
		data_iter = data_iter->next_data;
	}

	/* got to data length field */
	if (fseek(outstream, 2L, SEEK_SET)) {
		fprintf(stderr, "Could not set file position pointer to data length header!\n");
		return;
	}

	/* save data length in big endian format */
	for (i = 3; i >= 0; i--) {
		if (fputc(((data_length >> (i*8)) & 0xff), outstream) == EOF) {
			fprintf(stderr, "Could not write to file!\n");
			return;
		}
	}

	/* set file position pointer to EOF */
	if (fseek(outstream, 0L, SEEK_END)) {
		fprintf(stderr, "Could not set file position pointer to end of file!\n");
		return;
	}


}

/**
  * @brief Registers all generic functions of assembler object.
  * @param[in,out] assembler Pointer to assembler object which contains
  * all needed functions.
  * @return 0 on success, 1 otherwise.
  */
int gen_assembler_init(gen_assembler_t* assembler) {

	assembler->saveData = saveData;
	assembler->saveDataLabel = saveDataLabel;
	assembler->saveLabel = saveLabel;
	assembler->saveAddress = saveAddress;
	
	assembler->saveBranchInstr = saveBranchInstr;
	assembler->saveCallInstr = saveCallInstr;
	assembler->saveRegRegInstr = saveRegRegInstr;
	assembler->saveRegImmInstr = saveRegImmInstr;
	assembler->saveRegLabelInstr = saveRegLabelInstr;
	assembler->saveSethiInstr = saveSethiInstr;
	assembler->saveSethiLabelInstr = saveSethiLabelInstr;
	assembler->saveAddrInstr = saveAddrInstr;
	assembler->saveRdInstr = saveRdInstr;
	
	assembler->saveMovCCInstr = saveMovCCInstr;
	assembler->saveSelCCRegRegInstr = saveSelCCRegRegInstr;
	assembler->saveSelCCRegImmInstr = saveSelCCRegImmInstr;
	assembler->saveSelCCImmImmInstr = saveSelCCImmImmInstr;
	assembler->saveHWLoopStartInstr = saveZeroOperandInstr;
	assembler->saveHWLoopInitInstr = saveHWLoopInitInstr;
	assembler->saveHWLoopBoundRegInstr = saveHWLoopBoundRegInstr;
	assembler->saveHWLoopBoundImmInstr = saveHWLoopBoundImmInstr;
	assembler->savePredBeginInstr = saveZeroOperandInstr;
	assembler->savePredRegInstr = savePredRegInstr;
	assembler->savePredendInstr = saveZeroOperandInstr;
	assembler->saveSimulatorInstr = saveZeroOperandInstr;

	assembler->addICC = addICC;
	assembler->addPReg = addPReg;
	
	assembler->checkLabels = checkLabels;
	assembler->printData = printData;
	
	assembler->getFirstInstruction = getFirstInstruction;
	
	assembler->cleanUp = cleanUp;

	return 0;
}


