%{
/*
 * SPARC V8 Instruction Set Extension Simulator
 *
 * File: yy/sparc.l
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

#include <string.h>
#include "sparc_target.h"
#include "sparc.tab.h"

/** Externally defined string of current program name. */
extern char* progname;
/** Save the current line of input file. */
int yyline = 1;
/** Save the current column of input file. */
int yycolumn = 1;


%}

digit		[0-9]
letter		[a-zA-Z]
symbol		"["|"]"|","|"("|")"|"+"|"-"|":"|"\""|"@"
condcode	"a"|"n"|"ne"|"e"|"g"|"le"|"ge"|"l"|"gu"|"leu"|"cc"|"cs"|"pos"|"neg"|"vc"|"vs"

%%

	/* handle different kinds of special characters */

{symbol} {
	yycolumn += yyleng;
	return yytext[0];
}

	/* handle assembler meta information */

".file" {
	yycolumn += yyleng;
	return FILE_INFO;
}

".text" {
	yycolumn += yyleng;
	return TEXT_INFO;
}

".data" {
	yycolumn += yyleng;
	return DATA_INFO;
}

".section" {
	yycolumn += yyleng;
	return SECTION_INFO;
}

".rodata" {
	yycolumn += yyleng;
	return RODATA_INFO;
}

".bss" {
	yycolumn += yyleng;
	return BSS_INFO;
}

"#alloc" {
	yycolumn += yyleng;
	return ALLOC_INFO;
}

".data.rel.local" {
	yycolumn += yyleng;
	return DATA_REL_LOCAL_INFO;
}

"#write" {
	yycolumn += yyleng;
	return WRITE_INFO;
}

".globl" {
	yycolumn += yyleng;
	return GLOBL_INFO;
}

".align" {
	yycolumn += yyleng;
	return ALIGN_INFO;
}

".type" {
	yycolumn += yyleng;
	return TYPE_INFO;
}

".size" {
	yycolumn += yyleng;
	return SIZE_INFO;
}

"@function" {
	yycolumn += yyleng;
	yylval.string = (char *) strndup(&(yytext[1]), yyleng - 1);
	return TYPE_ARG;
}

"@object" {
	yycolumn += yyleng;
	yylval.string = (char *) strndup(&(yytext[1]), yyleng - 1);
	return TYPE_ARG;
}

".word" {
	yycolumn += yyleng;
	return WORD;
}

".half" {
	yycolumn += yyleng;
	return HALF;
}

".byte" {
	yycolumn += yyleng;
	return BYTE;
}

".skip" {
	yycolumn += yyleng;
	return SKIP;
}

	/* load integer instructions (section B.1.) */ 

"ldsb" {
	yycolumn += yyleng;
	yylval.value = LDSB;
	return LOAD;
}

"ldsh" {
	yycolumn += yyleng;
	yylval.value = LDSH;
	return LOAD;
}

"ldub" {
	yycolumn += yyleng;
	yylval.value = LDUB;
	return LOAD;
}

"lduh" {
	yycolumn += yyleng;
	yylval.value = LDUH;
	return LOAD;
}

"ld" {
	yycolumn += yyleng;
	yylval.value = LD;
	return LOAD;
}

"ldd" {
	yycolumn += yyleng;
	yylval.value = LDD;
	return LOAD;
}

"ldsba" {
	yycolumn += yyleng;
	yylval.value = LDSBA;
	return LOADA;
}

"ldsha" {
	yycolumn += yyleng;
	yylval.value = LDSHA;
	return LOADA;
}

"lduba" {
	yycolumn += yyleng;
	yylval.value = LDUBA;
	return LOADA;
}

"lduha" {
	yycolumn += yyleng;
	yylval.value = LDUHA;
	return LOADA;
}

"lda" {
	yycolumn += yyleng;
	yylval.value = LDA;
	return LOADA;
}

"ldda" {
	yycolumn += yyleng;
	yylval.value = LDDA;
	return LOADA;
}

	/* store integer instructions (section B.4.) */

"stb" {
	yycolumn += yyleng;
	yylval.value = STB;
	return STORE;
}

"sth" {
	yycolumn += yyleng;
	yylval.value = STH;
	return STORE;
}

"st" {
	yycolumn += yyleng;
	yylval.value = ST;
	return STORE;
}

"std" {
	yycolumn += yyleng;
	yylval.value = STD;
	return STORE;
}

"stba" {
	yycolumn += yyleng;
	yylval.value = STBA;
	return STOREA;
}

"stha" {
	yycolumn += yyleng;
	yylval.value = STHA;
	return STOREA;
}

"sta" {
	yycolumn += yyleng;
	yylval.value = STA;
	return STOREA;
}

"stda" {
	yycolumn += yyleng;
	yylval.value = STDA;
	return STOREA;
}

	/* atomic load/store instructions (section B.7.) */

"ldstub" {
	yycolumn += yyleng;
	yylval.value = LDSTUB;
	return LDSTA;
}

"ldstuba" {
	yycolumn += yyleng;
	yylval.value = LDSTUBA;
	return LDSTA;
}

	/* swap instructions (section B.8.) */

"swap" {
	yycolumn += yyleng;
	yylval.value = SWAP;
	return SWP;
}

"swapa" {
	yycolumn += yyleng;
	yylval.value = SWAPA;
	return SWP;
}

	/* sethi instruction (section B.9.) */

"sethi" {
	yycolumn += yyleng;
	return SETHI;
}
	
	/* nop instruction (section B.10.) */
	
"nop" {
	yycolumn += yyleng;
	return NOP;
}


	/* logical instruction (section B.11.) */

"and" {
	yycolumn += yyleng;
	yylval.value = AND;
	return LOGIC;
}

"andcc" {
	yycolumn += yyleng;
	yylval.value = ANDCC;
	return LOGIC;
}

"andn" {
	yycolumn += yyleng;
	yylval.value = ANDN;
	return LOGIC;
}

"andncc" {
	yycolumn += yyleng;
	yylval.value = ANDNCC;
	return LOGIC;
}

"or" {
	yycolumn += yyleng;
	yylval.value = OR;
	return LOGIC;
}

"orcc" {
	yycolumn += yyleng;
	yylval.value = ORCC;
	return LOGIC;
}

"orn" {
	yycolumn += yyleng;
	yylval.value = ORN;
	return LOGIC;
}

"orncc" {
	yycolumn += yyleng;
	yylval.value = ORNCC;
	return LOGIC;
}

"xor" {
	yycolumn += yyleng;
	yylval.value = XOR;
	return LOGIC;
}

"xorcc" {
	yycolumn += yyleng;
	yylval.value = XORCC;
	return LOGIC;
}

"xnor" {
	yycolumn += yyleng;
	yylval.value = XNOR;
	return LOGIC;
}

"xnorcc" {
	yycolumn += yyleng;
	yylval.value = XNORCC;
	return LOGIC;
}

	/* shift instructions (section B.12.) */

"sll" {
	yycolumn += yyleng;
	yylval.value = SLL;
	return SHIFT;
}

"srl" {
	yycolumn += yyleng;
	yylval.value = SRL;
	return SHIFT;
}

"sra" {
	yycolumn += yyleng;
	yylval.value = SRA;
	return SHIFT;
}

	/* add instructions (section B.13.) */

"add" {
	yycolumn += yyleng;
	yylval.value = ADD;
	return ARITHM;
}

"addcc" {
	yycolumn += yyleng;
	yylval.value = ADDCC;
	return ARITHM;
}

"addx" {
	yycolumn += yyleng;
	yylval.value = ADDX;
	return ARITHM;
}

"addxcc" {
	yycolumn += yyleng;
	yylval.value = ADDXCC;
	return ARITHM;
}

	/* tagged add instructions (section B.14.) */

"taddcc" {
	yycolumn += yyleng;
	yylval.value = TADDCC;
	return ARITHM;
}

"taddcctv" {
	yycolumn += yyleng;
	yylval.value = TADDCCTV;
	return ARITHM;
}

	/* subtract instructions (section B.15.) */

"sub" {
	yycolumn += yyleng;
	yylval.value = SUB;
	return ARITHM;
}

"subcc" {
	yycolumn += yyleng;
	yylval.value = SUBCC;
	return ARITHM;
}

"subx" {
	yycolumn += yyleng;
	yylval.value = SUBX;
	return ARITHM;
}

"subxcc" {
	yycolumn += yyleng;
	yylval.value = SUBXCC;
	return ARITHM;
}

	/* tagged subtract instructions (section B.16.) */

"tsubcc" {
	yycolumn += yyleng;
	yylval.value = TSUBCC;
	return ARITHM;
}

"tsubcctv" {
	yycolumn += yyleng;
	yylval.value = TSUBCCTV;
	return ARITHM;
}

	/* multiply step instruction (section B.17.) */

"mulscc" {
	yycolumn += yyleng;
	yylval.value = MULSCC;
	return ARITHM;
}

	/* multiply instructions (section B.18.) */

"umul" {
	yycolumn += yyleng;
	yylval.value = UMUL;
	return ARITHM;
}

"smul" {
	yycolumn += yyleng;
	yylval.value = SMUL;
	return ARITHM;
}

"umulcc" {
	yycolumn += yyleng;
	yylval.value = UMULCC;
	return ARITHM;
}

"smulcc" {
	yycolumn += yyleng;
	yylval.value = SMULCC;
	return ARITHM;
}

	/* divide instructions (section B.19.) */

"udiv" {
	yycolumn += yyleng;
	yylval.value = UDIV;
	return ARITHM;
}

"sdiv" {
	yycolumn += yyleng;
	yylval.value = SDIV;
	return ARITHM;
}

"udivcc" {
	yycolumn += yyleng;
	yylval.value = UDIVCC;
	return ARITHM;
}

"sdivcc" {
	yycolumn += yyleng;
	yylval.value = SDIVCC;
	return ARITHM;
}

	/* save and restore instructions (section B.20.) */

"save" {
	yycolumn += yyleng;
	yylval.value = SAVE;
	return SVREST;
}

"restore" {
	yycolumn += yyleng;
	yylval.value = RESTORE;
	return SVREST;
}

	/* branch instructions (section B.21.) */

"b"{condcode} {
	yycolumn += yyleng;
	if (!strcmp(yytext, "ba")) {
		yylval.value = CC_A;
	} else if (!strcmp(yytext, "bn")) {
		yylval.value = CC_N;
	} else if (!strcmp(yytext, "bne")) {
		yylval.value = CC_NE;
	} else if (!strcmp(yytext, "be")) {
		yylval.value = CC_E;
	} else if (!strcmp(yytext, "bg")) {
		yylval.value = CC_G;
	} else if (!strcmp(yytext, "ble")) {
		yylval.value = CC_LE;
	} else if (!strcmp(yytext, "bge")) {
		yylval.value = CC_GE;
	} else if (!strcmp(yytext, "bl")) {
		yylval.value = CC_L;
	} else if (!strcmp(yytext, "bgu")) {
		yylval.value = CC_GU;
	} else if (!strcmp(yytext, "bleu")) {
		yylval.value = CC_LEU;
	} else if (!strcmp(yytext, "bcc")) {
		yylval.value = CC_CC;
	} else if (!strcmp(yytext, "bcs")) {
		yylval.value = CC_CS;
	} else if (!strcmp(yytext, "bpos")) {
		yylval.value = CC_POS;
	} else if (!strcmp(yytext, "bneg")) {
		yylval.value = CC_NEG;
	} else if (!strcmp(yytext, "bvc")) {
		yylval.value = CC_VC;
	} else if (!strcmp(yytext, "bvs")) {
		yylval.value = CC_VS;
	}
	return BRANCH;
}

	/* call and link instruction (section B.24.) */

"call" {
	yycolumn += yyleng;
	return CALL;
}

	/* jump and link instruction (section B.25.) */

"jumpl" {
	yycolumn += yyleng;
	return JUMPL;
}

"jmp" {
	yycolumn += yyleng;
	return JMP;
}

	/* read state register instructions (section B.28.) */

"rd" {
	yycolumn += yyleng;
	return RD;
}
	
	/* write state register instructions (section B.29.) */

"wr" {
	yycolumn += yyleng;
	return WR;
}

	/* handle movcc and selcc */

"mov" {
	yycolumn += yyleng;
	return MOV;
}

"sel" {
	yycolumn += yyleng;
	return SEL;
}


	/* handle integer condition codes */

"["{condcode}"]" {
	yycolumn += yyleng;
	if (!strcmp(yytext, "[a]")) {
		yylval.value = CC_A;
	} else if (!strcmp(yytext, "[n]")) {
		yylval.value = CC_N;
	} else if (!strcmp(yytext, "[ne]")) {
		yylval.value = CC_NE;
	} else if (!strcmp(yytext, "[e]")) {
		yylval.value = CC_E;
	} else if (!strcmp(yytext, "[g]")) {
		yylval.value = CC_G;
	} else if (!strcmp(yytext, "[le]")) {
		yylval.value = CC_LE;
	} else if (!strcmp(yytext, "[ge]")) {
		yylval.value = CC_GE;
	} else if (!strcmp(yytext, "[l]")) {
		yylval.value = CC_L;
	} else if (!strcmp(yytext, "[gu]")) {
		yylval.value = CC_GU;
	} else if (!strcmp(yytext, "[leu]")) {
		yylval.value = CC_LEU;
	} else if (!strcmp(yytext, "[cc]")) {
		yylval.value = CC_CC;
	} else if (!strcmp(yytext, "[cs]")) {
		yylval.value = CC_CS;
	} else if (!strcmp(yytext, "[vc]")) {
		yylval.value = CC_VC;
	} else if (!strcmp(yytext, "[vs]")) {
		yylval.value = CC_VS;
	}
	return ICC;
}

	/* handle t/f flags for predicated instructions */

"["(t|f)"]" {
	yycolumn += yyleng;
	if (yytext[1] == 't') {
		yylval.value = 1;
	} else {
		yylval.value = 0;
	}
	return PRED_REG_TF;
}

	/* predicated blocks */

"predbegin" {
	yycolumn += yyleng;
	return PREDBEGIN;
}

"predend" {
	yycolumn += yyleng;
	return PREDEND;
}

	/* instructions for predicate registers */

"predset" {
	yycolumn += yyleng;
	return PREDSET;
}

"predclear" {
	yycolumn += yyleng;
	return PREDCLEAR;
}

	/* handle hardware loop instructions */

"hwloop init" {
	yycolumn += yyleng;
	return HWLOOP_INIT;
}

"hwloop start" {
	yycolumn += yyleng;
	return HWLOOP_START;
}

"%loops" {
	yycolumn += yyleng;
	return LOOPS_REGISTER;
}

"%loope" {
	yycolumn += yyleng;
	return LOOPE_REGISTER;
}

"%loopb" {
	yycolumn += yyleng;
	return LOOPB_REGISTER;
}

	/* handle decimal integers */

"-"?(([1-9]{digit}*)|("0")) {
	yycolumn += yyleng;
	yylval.value = (int) ((uint32_t) strtoll(yytext, NULL, 10));
	return IMMEDIATE;
}

	/* handle all kind of registers */

"%"g[0-7] {
	yycolumn += yyleng;
	yylval.value = yytext[yyleng-1] - (int) '0' + G_REGISTER;
	return REGISTER;
}

"%"o[0-7] {
	yycolumn += yyleng;
	yylval.value = yytext[yyleng-1] - (int) '0' + O_REGISTER;
	return REGISTER;
}

"%"l[0-7] {
	yycolumn += yyleng;
	yylval.value = yytext[yyleng-1] - (int) '0' + L_REGISTER;
	return REGISTER;
}

"%"i[0-7] {
	yycolumn += yyleng;
	yylval.value = yytext[yyleng-1] - (int) '0' + I_REGISTER;
	return REGISTER;
}

"%"p([0-9]|(1[0-5])) {
	yycolumn += yyleng;
	yylval.value = (int) strtol(&yytext[2], NULL, 10);
	return P_REGISTER;
}

"%sp" {
	yycolumn += yyleng;
	yylval.value = SP_REGISTER;
	return REGISTER;
}

"%fp" {
	yycolumn += yyleng;
	yylval.value = FP_REGISTER;
	return REGISTER;
}

"%y" {
	yycolumn += yyleng;
	yylval.value = Y_REGISTER_NO;
	return Y_REGISTER;
}

	/* handle hi and low assembler instructions */

"%hi" {
	yycolumn += yyleng;
	return HI;
}

"%lo" {
	yycolumn += yyleng;
	return LOW;
}

	/* handle simulator instructions */

"sim-printcycles" {
	yycolumn += yyleng;
	return CYCLE_PRINT;
}

"sim-clearcycles" {
	yycolumn += yyleng;
	return CYCLE_CLEAR;
}

	/* handle labels */

"."?{letter}({digit}|{letter}|"#"|"."|"_")* {
	yycolumn += yyleng;
	yylval.string = (char *) strndup(yytext, yyleng);
	return LABEL;
}

	/* ignore comments until end of line */

"!".*$ {
}

	/* eat up whitespaces */

[ \t]+ {
	yycolumn += yyleng;
}

	/* handle new lines */

[\n]+ {
	yyline += yyleng;
	yycolumn = 1;
}

	/* handle everything else */

. {
	fprintf(stderr, "%s: Unknown identifier \"%s\" in line %d, column %d.\n", 
		progname, yytext, yyline, yycolumn);
	exit(EXIT_FAILURE); 
}
