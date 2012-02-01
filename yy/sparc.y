%{
/*
 * SPARC V8 Instruction Set Extension Simulator
 *
 * File: yy/sparc.y
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
#include "sparc_target.h"
#include "gen_assembler.h"

/** Externally defined line number of current input file. */
extern int yyline;
/** Externally defined column number of current input file. */
extern int yycolumn;
/** Externally defined name of current program. */
extern char* progname;
/** Externally defined assembler struct which provides all printing functions. */
extern gen_assembler_t* assembler;


/** Externally defined output file stream of yacc. */
extern FILE* yyout;
/** Externally defined input file stream of yacc. */
extern FILE* yyin;

/** Save the instruction number. */
unsigned instr_no = 0;

/** Save the data memory counter. */
unsigned data_no = 0;

int section = SECTION_UNDEF;

/**
  * @brief Prints out the given error message and exits the program.
  * @param[in] e The specific error message to be printed out.
  */
int yyerror(char* e) { 
    fprintf(stderr, "%s: Parsing error in line %d, column %d.\n%s\n", 
		progname, yyline, yycolumn - 1, e);
	assembler->cleanUp();
	/* close open file handles */
	if (yyin != stdin) {
		fclose(yyin);
	}
	if (yyout != stdout) {
		fclose(stdout);
	}
    exit(EXIT_FAILURE);
}


%}

%union {
	int value;
	char* string;
	sparc_address* address;
};

%token<string> LABEL TYPE_ARG

/* register and condition codes */
%token<value> REGISTER IMMEDIATE Y_REGISTER P_REGISTER LOOPS_REGISTER LOOPE_REGISTER LOOPB_REGISTER ICC PRED_REG_TF

/* assembler meta information */
%token FILE_INFO TEXT_INFO GLOBL_INFO ALIGN_INFO TYPE_INFO SIZE_INFO SECTION_INFO DATA_INFO RODATA_INFO BSS_INFO 
%token ALLOC_INFO DATA_REL_LOCAL_INFO WRITE_INFO
%token WORD HALF BYTE SKIP HI LOW

/* simulator meta information */
%token CYCLE_PRINT CYCLE_CLEAR

/* generic instruction tokens */
%token<value> LOAD LOADA STORE STOREA LDSTA SWP LOGIC SHIFT ARITHM SVREST BRANCH 

/* specific instruction tokens */
/* load instructions */ 
%token LDSB LDSH LDUB LDUH LD LDD LDSBA LDSHA LDUBA LDUHA LDA LDDA
/* store instructions */
%token STB STH ST STD STBA STHA STA STDA
/* atomic load/store instructions */
%token LDSTUB LDSTUBA
/* swap instructions */
%token SWAP SWAPA
/* logical instructions */
%token AND ANDCC ANDN ANDNCC OR ORCC ORN ORNCC XOR XORCC XNOR XNORCC
/* shift instructions */
%token SLL SRL SRA
/* add instructions */
%token ADD ADDCC ADDX ADDXCC
/* tagged add instructions */
%token TADDCC TADDCCTV
/* subtract instructions */
%token SUB SUBCC SUBX SUBXCC
/* tagged subtract instructions */
%token TSUBCC TSUBCCTV
/* multiplay step instruction */
%token MULSCC 
/* multiplay instructions */
%token UMUL SMUL UMULCC SMULCC
/* divide instructions */
%token UDIV SDIV UDIVCC SDIVCC
/* save and restore instructions */
%token SAVE	RESTORE

/* non-generic instructions */
%token SETHI NOP CALL JUMPL JMP RD WR MOV SEL HWLOOP_INIT HWLOOP_START PREDBEGIN PREDEND PREDSET PREDCLEAR

/* unknown instruction */
%token UNKNOWN

%start assembler_prog

%%

assembler_prog: /* empty program */
	| assembler_prog metainfo
	| assembler_prog branchtargetdefinition
	| assembler_prog data
	| assembler_prog instruction { instr_no++; }
	;

metainfo: FILE_INFO '"' LABEL '"'
	| TEXT_INFO { section = SECTION_TEXT; }
	| DATA_INFO { section = SECTION_DATA; }
	| SECTION_INFO RODATA_INFO ',' ALLOC_INFO { section = SECTION_DATA; }
	| SECTION_INFO DATA_REL_LOCAL_INFO ',' ALLOC_INFO ',' WRITE_INFO { section = SECTION_DATA; }
	| SECTION_INFO BSS_INFO ',' ALLOC_INFO ',' WRITE_INFO { section = SECTION_DATA; }
	| GLOBL_INFO LABEL
	| ALIGN_INFO IMMEDIATE { if ($2 < 0 || $2 > 32) {yyerror("Unknown alignment number!");} }
	| TYPE_INFO LABEL ',' TYPE_ARG { /* fprintf(stderr, "Label %s - Type: %s\n", $2, $4); */ } 
	| SIZE_INFO LABEL ',' LABEL '-' LABEL
	| SIZE_INFO LABEL ',' IMMEDIATE
	;

branchtargetdefinition: LABEL ':' 
	{
		if (section == SECTION_TEXT) { 
			if (instr_no == 0 && (strcmp($1, "main"))) {
				yyerror("First instruction label has to be main!");
			}
			assembler->saveLabel(instr_no, $1); 
		} else if (section == SECTION_DATA) {
			assembler->saveLabel(data_no, $1);
		} else {
			yyerror("Label was found in unknown section!");
		}
	}
	;

data: WORD IMMEDIATE { assembler->saveData(data_no, $2, 4); data_no += 4; }
	| WORD LABEL {assembler->saveDataLabel(data_no, $2, 4); data_no += 4; }
	| HALF IMMEDIATE { assembler->saveData(data_no, $2, 2); data_no += 2; }
	| BYTE IMMEDIATE { assembler->saveData(data_no, $2, 1); data_no += 1; }
	| SKIP IMMEDIATE { data_no += $2; }
	;

instruction: loadinstructions
	| storeinstructions
	| atomicloadstoreinstructions
	| swapinstructions
	| sethiinstruction
	| nopinstruction
	| logicalinstructions
	| shiftinstructions
	| arithmeticinstructions
	| saverestoreinstructions
	| branchinstructions
	| callinstruction
	| jumplinkinstruction
	| readstatusregisterinstruction
	| writestatusregisterinstruction
	| movccinstructions
	{
		if (!(assembler->hasMovCC())) { 
			yyerror("Target does not support conditional moves!"); 
		} 
	}
	| selccinstructions
	{
		if (!(assembler->hasSelCC())) { 
			yyerror("Target does not support conditional selects!"); 
		} 
	}
	| hwloopinstructions
	{
		if (!(assembler->hasHWLoops())) {
			yyerror("Target does not support hardware loops!");
		}
	}
	| predicatedblocksinstructions
	| predicatedreginstructions
	{
		if (!(assembler->hasPredBlocksReg()) && !(assembler->hasPredInstrsReg())) {
			yyerror("Target does not support predicate registers!");
		}
	}
	| CYCLE_PRINT { assembler->saveSimulatorInstr(instr_no, CYCLE_PRINT);}
	| CYCLE_CLEAR { assembler->saveSimulatorInstr(instr_no, CYCLE_CLEAR);}
	; 

loadinstructions: LOAD '[' addressdefinition ']' ',' REGISTER { assembler->saveAddrInstr(instr_no, $1, $6, $<address>3); }
	| LOAD ICC '[' addressdefinition ']' ',' REGISTER
	{
		if (!(assembler->hasPredInstrsCC())) {
			yyerror("Target does not support predicate instructions on condition codes!");
		}
		assembler->saveAddrInstr(instr_no, $1, $7, $<address>4);
		assembler->addICC(instr_no, $2);
	}
	| LOAD '[' P_REGISTER ']' PRED_REG_TF '[' addressdefinition ']' ',' REGISTER
	{
		if (!(assembler->hasPredInstrsReg())) {
			yyerror("Target does not support predicate instructions on predicate registers!");
		}
		assembler->saveAddrInstr(instr_no, $1, $10, $<address>7);
		assembler->addPReg(instr_no, $3, $5);
	}
	;

storeinstructions: STORE REGISTER ',' '[' addressdefinition ']' { assembler->saveAddrInstr(instr_no, $1, $2, $<address>5); }
	| STORE ICC REGISTER ',' '[' addressdefinition ']'
	{
		if (!(assembler->hasPredInstrsCC())) {
			yyerror("Target does not support predicate instructions on condition codes!");
		}
		assembler->saveAddrInstr(instr_no, $1, $3, $<address>6);
		assembler->addICC(instr_no, $2);
	}
	| STORE '[' P_REGISTER ']' PRED_REG_TF REGISTER ',' '[' addressdefinition ']'
	{
		if (!(assembler->hasPredInstrsReg())) {
			yyerror("Target does not support predicate instructions on predicate registers!");
		}
		assembler->saveAddrInstr(instr_no, $1, $6, $<address>9);
		assembler->addPReg(instr_no, $3, $5);
	}
	;

atomicloadstoreinstructions: LDSTA '[' addressdefinition ']' ',' REGISTER { assembler->saveAddrInstr(instr_no, $1, $6, $<address>3); }
	| LDSTA ICC '[' addressdefinition ']' ',' REGISTER
	{
		if (!(assembler->hasPredInstrsCC())) {
			yyerror("Target does not support predicate instructions on condition codes!");
		}
		assembler->saveAddrInstr(instr_no, $1, $7, $<address>4);
		assembler->addICC(instr_no, $2);
	}
	| LDSTA '[' P_REGISTER ']' PRED_REG_TF '[' addressdefinition ']' ',' REGISTER
	{
		if (!(assembler->hasPredInstrsReg())) {
			yyerror("Target does not support predicate instructions on predicate registers!");
		}
		assembler->saveAddrInstr(instr_no, $1, $10, $<address>7);
		assembler->addPReg(instr_no, $3, $5);
	}
	;

swapinstructions: SWP '[' addressdefinition ']' ',' REGISTER { assembler->saveAddrInstr(instr_no, $1, $6, $<address>3); }
	| SWP ICC '[' addressdefinition ']' ',' REGISTER
	{
		if (!(assembler->hasPredInstrsCC())) {
			yyerror("Target does not support predicate instructions on condition codes!");
		}
		assembler->saveAddrInstr(instr_no, $1, $7, $<address>4);
		assembler->addICC(instr_no, $2);
	}
	| SWP '[' P_REGISTER ']' PRED_REG_TF '[' addressdefinition ']' ',' REGISTER
	{
		if (!(assembler->hasPredInstrsReg())) {
			yyerror("Target does not support predicate instructions on predicate registers!");
		}
		assembler->saveAddrInstr(instr_no, $1, $10, $<address>7);
		assembler->addPReg(instr_no, $3, $5);
	}
	;

sethiinstruction: SETHI IMMEDIATE ',' REGISTER { assembler->saveSethiInstr(instr_no, SETHI, $4, $2); }
	| SETHI HI '(' LABEL ')' ',' REGISTER { assembler->saveSethiLabelInstr(instr_no, SETHI, $7, $4); }
	| SETHI ICC IMMEDIATE ',' REGISTER
	{
		if (!(assembler->hasPredInstrsCC())) {
			yyerror("Target does not support predicate instructions on condition codes!");
		}
		assembler->saveSethiInstr(instr_no, SETHI, $5, $3);
		assembler->addICC(instr_no, $2);
	}
	| SETHI ICC HI '(' LABEL ')' ',' REGISTER 
	{
		if (!(assembler->hasPredInstrsCC())) {
			yyerror("Target does not support predicate instructions on condition codes!");
		}
		assembler->saveSethiLabelInstr(instr_no, SETHI, $8, $5);
		assembler->addICC(instr_no, $2);
	}
	| SETHI '[' P_REGISTER ']' PRED_REG_TF IMMEDIATE ',' REGISTER
	{
		if (!(assembler->hasPredInstrsReg())) {
			yyerror("Target does not support predicate instructions on predicate registers!");
		}
		assembler->saveSethiInstr(instr_no, SETHI, $8, $6);
		assembler->addPReg(instr_no, $3, $5);
	}
	| SETHI '[' P_REGISTER ']' PRED_REG_TF HI '(' LABEL ')' ',' REGISTER
	{
		if (!(assembler->hasPredInstrsReg())) {
			yyerror("Target does not support predicate instructions on predicate registers!");
		}
		assembler->saveSethiLabelInstr(instr_no, SETHI, $11, $8);
		assembler->addPReg(instr_no, $3, $5);
	}
	;

nopinstruction: NOP { assembler->saveSethiInstr(instr_no, SETHI, G_REGISTER + 0, 0); }
	;

logicalinstructions: LOGIC REGISTER ',' REGISTER ',' REGISTER { assembler->saveRegRegInstr(instr_no, $1, $6, $2, $4); }
	| LOGIC REGISTER ',' IMMEDIATE ',' REGISTER { assembler->saveRegImmInstr(instr_no, $1, $6, $2, $4); }
	| LOGIC REGISTER ',' LOW '(' LABEL ')' ',' REGISTER { assembler->saveRegLabelInstr(instr_no, $1, $9, $2, $6); }
	| LOGIC ICC REGISTER ',' REGISTER ',' REGISTER
	{
		if (!(assembler->hasPredInstrsCC())) {
			yyerror("Target does not support predicate instructions on condition codes!");
		}
		assembler->saveRegRegInstr(instr_no, $1, $7, $3, $5);
		assembler->addICC(instr_no, $2);
	}
	| LOGIC ICC REGISTER ',' IMMEDIATE ',' REGISTER
	{
		if (!(assembler->hasPredInstrsCC())) {
			yyerror("Target does not support predicate instructions on condition codes!");
		}
		assembler->saveRegImmInstr(instr_no, $1, $7, $3, $5);
		assembler->addICC(instr_no, $2);
	}
	| LOGIC ICC REGISTER ',' LOW '(' LABEL ')' ',' REGISTER 
	{ 
		if (!(assembler->hasPredInstrsCC())) {
			yyerror("Target does not support predicate instructions on condition codes!");
		}
		assembler->saveRegLabelInstr(instr_no, $1, $10, $3, $7);
		assembler->addICC(instr_no, $2);
	}
	| LOGIC '[' P_REGISTER ']' PRED_REG_TF REGISTER ',' REGISTER ',' REGISTER
	{
		if (!(assembler->hasPredInstrsReg())) {
			yyerror("Target does not support predicate instructions on predicate registers!");
		}
		assembler->saveRegRegInstr(instr_no, $1, $10, $6, $8);
		assembler->addPReg(instr_no, $3, $5);
	}
	| LOGIC '[' P_REGISTER ']' PRED_REG_TF REGISTER ',' IMMEDIATE ',' REGISTER 
	{
		if (!(assembler->hasPredInstrsReg())) {
			yyerror("Target does not support predicate instructions on predicate registers!");
		}
		assembler->saveRegImmInstr(instr_no, $1, $10, $6, $8);
		assembler->addPReg(instr_no, $3, $5);
	}
	| LOGIC '[' P_REGISTER ']' PRED_REG_TF REGISTER ',' LOW '(' LABEL ')' ',' REGISTER 
	{
		if (!(assembler->hasPredInstrsReg())) {
			yyerror("Target does not support predicate instructions on predicate registers!");
		}
		assembler->saveRegLabelInstr(instr_no, $1, $13, $6, $10);
		assembler->addPReg(instr_no, $3, $5);
	}
	;

shiftinstructions: SHIFT REGISTER ',' REGISTER ',' REGISTER { assembler->saveRegRegInstr(instr_no, $1, $6, $2, $4); }
	| SHIFT REGISTER ',' IMMEDIATE ',' REGISTER { assembler->saveRegImmInstr(instr_no, $1, $6, $2, $4); }
	| SHIFT REGISTER ',' LOW '(' LABEL ')' ',' REGISTER { assembler->saveRegLabelInstr(instr_no, $1, $9, $2, $6); }
	| SHIFT ICC REGISTER ',' REGISTER ',' REGISTER
	{
		if (!(assembler->hasPredInstrsCC())) {
			yyerror("Target does not support predicate instructions on condition codes!");
		}
		assembler->saveRegRegInstr(instr_no, $1, $7, $3, $5);
		assembler->addICC(instr_no, $2);
	}
	| SHIFT ICC REGISTER ',' IMMEDIATE ',' REGISTER
	{
		if (!(assembler->hasPredInstrsCC())) {
			yyerror("Target does not support predicate instructions on condition codes!");
		}
		assembler->saveRegImmInstr(instr_no, $1, $7, $3, $5);
		assembler->addICC(instr_no, $2);
	}
	| SHIFT ICC REGISTER ',' LOW '(' LABEL ')' ',' REGISTER 
	{ 
		if (!(assembler->hasPredInstrsCC())) {
			yyerror("Target does not support predicate instructions on condition codes!");
		}
		assembler->saveRegLabelInstr(instr_no, $1, $10, $3, $7);
		assembler->addICC(instr_no, $2);
	}
	| SHIFT '[' P_REGISTER ']' PRED_REG_TF REGISTER ',' REGISTER ',' REGISTER
	{
		if (!(assembler->hasPredInstrsReg())) {
			yyerror("Target does not support predicate instructions on predicate registers!");
		}
		assembler->saveRegRegInstr(instr_no, $1, $10, $6, $8);
		assembler->addPReg(instr_no, $3, $5);
	}
	| SHIFT '[' P_REGISTER ']' PRED_REG_TF REGISTER ',' IMMEDIATE ',' REGISTER
	{
		if (!(assembler->hasPredInstrsReg())) {
			yyerror("Target does not support predicate instructions on predicate registers!");
		}
		assembler->saveRegImmInstr(instr_no, $1, $10, $6, $8);
		assembler->addPReg(instr_no, $3, $5);
	}
	| SHIFT '[' P_REGISTER ']' PRED_REG_TF REGISTER ',' LOW '(' LABEL ')' ',' REGISTER 
	{
		if (!(assembler->hasPredInstrsReg())) {
			yyerror("Target does not support predicate instructions on predicate registers!");
		}
		assembler->saveRegLabelInstr(instr_no, $1, $13, $6, $10);
		assembler->addPReg(instr_no, $3, $5);
	}
	;

arithmeticinstructions: ARITHM REGISTER ',' REGISTER ',' REGISTER { assembler->saveRegRegInstr(instr_no, $1, $6, $2, $4); }
	| ARITHM REGISTER ',' IMMEDIATE ',' REGISTER { assembler->saveRegImmInstr(instr_no, $1, $6, $2, $4); }
	| ARITHM REGISTER ',' LOW '(' LABEL ')' ',' REGISTER { assembler->saveRegLabelInstr(instr_no, $1, $9, $2, $6); }
	| ARITHM ICC REGISTER ',' REGISTER ',' REGISTER
	{
		if (!(assembler->hasPredInstrsCC())) {
			yyerror("Target does not support predicate instructions on condition codes!");
		}
		assembler->saveRegRegInstr(instr_no, $1, $7, $3, $5);
		assembler->addICC(instr_no, $2);
	}
	| ARITHM ICC REGISTER ',' IMMEDIATE ',' REGISTER
	{
		if (!(assembler->hasPredInstrsCC())) {
			yyerror("Target does not support predicate instructions on condition codes!");
		}
		assembler->saveRegImmInstr(instr_no, $1, $7, $3, $5);
		assembler->addICC(instr_no, $2);
	}
	| ARITHM ICC REGISTER ',' LOW '(' LABEL ')' ',' REGISTER 
	{ 
		if (!(assembler->hasPredInstrsCC())) {
			yyerror("Target does not support predicate instructions on condition codes!");
		}
		assembler->saveRegLabelInstr(instr_no, $1, $10, $3, $7);
		assembler->addICC(instr_no, $2);
	}
	| ARITHM '[' P_REGISTER ']' PRED_REG_TF REGISTER ',' REGISTER ',' REGISTER
	{
		if (!(assembler->hasPredInstrsReg())) {
			yyerror("Target does not support predicate instructions on predicate registers!");
		}
		assembler->saveRegRegInstr(instr_no, $1, $10, $6, $8);
		assembler->addPReg(instr_no, $3, $5);
	}
	| ARITHM '[' P_REGISTER ']' PRED_REG_TF REGISTER ',' IMMEDIATE ',' REGISTER
	{
		if (!(assembler->hasPredInstrsReg())) {
			yyerror("Target does not support predicate instructions on predicate registers!");
		}
		assembler->saveRegImmInstr(instr_no, $1, $10, $6, $8);
		assembler->addPReg(instr_no, $3, $5);
	}
	| ARITHM '[' P_REGISTER ']' PRED_REG_TF REGISTER ',' LOW '(' LABEL ')' ',' REGISTER 
	{
		if (!(assembler->hasPredInstrsReg())) {
			yyerror("Target does not support predicate instructions on predicate registers!");
		}
		assembler->saveRegLabelInstr(instr_no, $1, $13, $6, $10);
		assembler->addPReg(instr_no, $3, $5);
	}
	;

saverestoreinstructions: SVREST REGISTER ',' REGISTER ',' REGISTER { assembler->saveRegRegInstr(instr_no, $1, $6, $2, $4); }
	| SVREST REGISTER ',' IMMEDIATE ',' REGISTER { assembler->saveRegImmInstr(instr_no, $1, $6, $2, $4); }
	| SVREST REGISTER ',' LOW '(' LABEL ')' ',' REGISTER { assembler->saveRegLabelInstr(instr_no, $1, $9, $2, $6); }
	| SVREST ICC REGISTER ',' REGISTER ',' REGISTER
	{
		if (!(assembler->hasPredInstrsCC())) {
			yyerror("Target does not support predicate instructions on condition codes!");
		}
		assembler->saveRegRegInstr(instr_no, $1, $7, $3, $5);
		assembler->addICC(instr_no, $2);
	}
	| SVREST ICC REGISTER ',' IMMEDIATE ',' REGISTER
	{
		if (!(assembler->hasPredInstrsCC())) {
			yyerror("Target does not support predicate instructions on condition codes!");
		}
		assembler->saveRegImmInstr(instr_no, $1, $7, $3, $5);
		assembler->addICC(instr_no, $2);
	}
	| SVREST ICC REGISTER ',' LOW '(' LABEL ')' ',' REGISTER 
	{ 
		if (!(assembler->hasPredInstrsCC())) {
			yyerror("Target does not support predicate instructions on condition codes!");
		}
		assembler->saveRegLabelInstr(instr_no, $1, $10, $3, $7);
		assembler->addICC(instr_no, $2);
	}
	| SVREST '[' P_REGISTER ']' PRED_REG_TF REGISTER ',' REGISTER ',' REGISTER
	{
		if (!(assembler->hasPredInstrsCC())) {
			yyerror("Target does not support predicate instructions on predicate registers!");
		}
		assembler->saveRegRegInstr(instr_no, $1, $10, $6, $8);
		assembler->addPReg(instr_no, $3, $5);
	}
	| SVREST '[' P_REGISTER ']' PRED_REG_TF REGISTER ',' IMMEDIATE ',' REGISTER
	{
		if (!(assembler->hasPredInstrsCC())) {
			yyerror("Target does not support predicate instructions on predicate registers!");
		}
		assembler->saveRegImmInstr(instr_no, $1, $10, $6, $8);
		assembler->addPReg(instr_no, $3, $5);
	}
	| SVREST '[' P_REGISTER ']' PRED_REG_TF REGISTER ',' LOW '(' LABEL ')' ',' REGISTER 
	{
		if (!(assembler->hasPredInstrsReg())) {
			yyerror("Target does not support predicate instructions on predicate registers!");
		}
		assembler->saveRegLabelInstr(instr_no, $1, $13, $6, $10);
		assembler->addPReg(instr_no, $3, $5);
	}
	;

branchinstructions: BRANCH LABEL { assembler->saveBranchInstr(instr_no, BRANCH, $1, $2); }
	;

callinstruction: CALL LABEL { assembler->saveCallInstr(instr_no, CALL, $2); }
	;

jumplinkinstruction: JUMPL addressdefinition ',' REGISTER { assembler->saveAddrInstr(instr_no, JUMPL, $4, $<address>2); }
	| JMP addressdefinition { assembler->saveAddrInstr(instr_no, JUMPL, G_REGISTER + 0, $<address>2); }
	| JUMPL ICC addressdefinition ',' REGISTER
	{
		if (!(assembler->hasPredInstrsCC())) {
			yyerror("Target does not support predicate instructions on condition codes!");
		}
		assembler->saveAddrInstr(instr_no, JUMPL, $5, $<address>3);
		assembler->addICC(instr_no, $2);
	}
	| JMP ICC addressdefinition
	{
		if (!(assembler->hasPredInstrsCC())) {
			yyerror("Target does not support predicate instructions on condition codes!");
		}
		assembler->saveAddrInstr(instr_no, JUMPL, G_REGISTER + 0, $<address>3);
		assembler->addICC(instr_no, $2);
	}
	| JUMPL '[' P_REGISTER ']' PRED_REG_TF addressdefinition ',' REGISTER
	{
		if (!(assembler->hasPredInstrsReg())) {
			yyerror("Target does not support predicate instructions on predicate registers!");
		}
		assembler->saveAddrInstr(instr_no, JUMPL, $8, $<address>6);
		assembler->addPReg(instr_no, $3, $5);
	}
	| JMP '[' P_REGISTER ']' PRED_REG_TF addressdefinition
	{
		if (!(assembler->hasPredInstrsReg())) {
			yyerror("Target does not support predicate instructions on predicate registers!");
		}
		assembler->saveAddrInstr(instr_no, JUMPL, G_REGISTER + 0, $<address>6);
		assembler->addPReg(instr_no, $3, $5);
	}
	;

readstatusregisterinstruction: RD Y_REGISTER ',' REGISTER { assembler->saveRdInstr(instr_no, RD, $4, $2); }
	| RD ICC Y_REGISTER ',' REGISTER
	{
		if (!(assembler->hasPredInstrsCC())) {
			yyerror("Target does not support predicate instructions on condition codes!");
		}
		assembler->saveRdInstr(instr_no, RD, $5, $2);
		assembler->addICC(instr_no, $2);
	}
	| RD '[' P_REGISTER ']' PRED_REG_TF Y_REGISTER ',' REGISTER
	{
		if (!(assembler->hasPredInstrsReg())) {
			yyerror("Target does not support predicate instructions on predicate registers!");
		}
		assembler->saveRdInstr(instr_no, RD, $8, $6);
		assembler->addPReg(instr_no, $3, $5);
	}
	;

writestatusregisterinstruction: WR REGISTER ',' REGISTER ',' Y_REGISTER { assembler->saveRegRegInstr(instr_no, WR, $6, $2, $4); }
	| WR REGISTER ',' IMMEDIATE ',' Y_REGISTER { assembler->saveRegImmInstr(instr_no, WR, $6, $2, $4); }
	;

selccinstructions: SEL ICC IMMEDIATE ',' IMMEDIATE ',' REGISTER { assembler->saveSelCCImmImmInstr(instr_no, SEL, $7, $3, $5, $2); }
	| SEL ICC REGISTER ',' IMMEDIATE ',' REGISTER { assembler->saveSelCCRegImmInstr(instr_no, SEL, $7, $3, $5, $2); }
	| SEL ICC REGISTER ',' REGISTER ',' REGISTER { assembler->saveSelCCRegRegInstr(instr_no, SEL, $7, $3, $5, $2); }
	;

movccinstructions: MOV ICC REGISTER ',' REGISTER { assembler->saveMovCCInstr(instr_no, MOV, $5, $3, $2); }
	;

hwloopinstructions: HWLOOP_INIT LABEL ',' LOOPS_REGISTER { assembler->saveHWLoopInitInstr(instr_no, HWLOOP_INIT, LOOPS_REGISTER, $2); }
	| HWLOOP_INIT LABEL ',' LOOPE_REGISTER { assembler->saveHWLoopInitInstr(instr_no, HWLOOP_INIT, LOOPE_REGISTER, $2); }
	| HWLOOP_INIT REGISTER ',' LOOPB_REGISTER { assembler->saveHWLoopBoundRegInstr(instr_no, HWLOOP_INIT, LOOPB_REGISTER, $2); } 
	| HWLOOP_INIT IMMEDIATE ',' LOOPB_REGISTER  { assembler->saveHWLoopBoundImmInstr(instr_no, HWLOOP_INIT, LOOPB_REGISTER, $2); } 
	| HWLOOP_START { assembler->saveHWLoopStartInstr(instr_no, HWLOOP_START); }
	;

predicatedblocksinstructions: PREDBEGIN ICC
	{
		if (!(assembler->hasPredBlocksCC())) {
			yyerror("Target does not support predicated blocks on condition codes!");
		}
		assembler->savePredBeginInstr(instr_no, PREDBEGIN);
		assembler->addICC(instr_no, $2);
	}
	| PREDBEGIN '[' P_REGISTER ']' PRED_REG_TF
	{
		if (!(assembler->hasPredBlocksReg())) {
			yyerror("Target does not support predicated blocks on predicate registers!");
		}
		assembler->savePredBeginInstr(instr_no, PREDBEGIN);
		assembler->addPReg(instr_no, $3, $5);
	}
	| PREDEND { assembler->savePredendInstr(instr_no, PREDEND); }
	;
	
predicatedreginstructions: PREDSET P_REGISTER { assembler->savePredRegInstr(instr_no, PREDSET, $2); }
	| PREDSET ICC P_REGISTER
	{
		assembler->savePredRegInstr(instr_no, PREDSET, $3);
		assembler->addICC(instr_no, $2);
	}
	| PREDSET '[' P_REGISTER ']' PRED_REG_TF ICC P_REGISTER
	{
		if (!(assembler->hasPredInstrsReg())) {
			yyerror("Target does not support predicated instructions on predicate registers!");
		}
		assembler->savePredRegInstr(instr_no, PREDSET, $7);
		assembler->addICC(instr_no, $6);
		assembler->addPReg(instr_no, $3, $5);
	}
	| PREDCLEAR P_REGISTER { assembler->savePredRegInstr(instr_no, PREDCLEAR, $2); }
	;

addressdefinition: REGISTER { $<address>$ = assembler->saveAddress($1, OPERAND_TYPE_REGISTER, (void*) 0, OPERAND_TYPE_SIMM13); }
	| REGISTER '+' REGISTER { $<address>$ = assembler->saveAddress($1, OPERAND_TYPE_REGISTER, (void*) $3, OPERAND_TYPE_REGISTER); }
	| REGISTER '+' IMMEDIATE { $<address>$ = assembler->saveAddress($1, OPERAND_TYPE_REGISTER, (void*) $3, OPERAND_TYPE_SIMM13); }
	| REGISTER '+' LOW '('LABEL ')' { $<address>$ = assembler->saveAddress($1, OPERAND_TYPE_REGISTER, (void*) $5, OPERAND_TYPE_LOW_LABEL); }
	;

%%

