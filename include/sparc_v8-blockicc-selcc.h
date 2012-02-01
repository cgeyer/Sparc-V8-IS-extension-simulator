/*
 * SPARC V8 Instruction Set Extension Simulator
 *
 * File: include/sparc_v8-blockicc-selcc.h
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

#ifndef __SPARC_V8_BLOCKICC_SELCC_H__
#define __SPARC_V8_BLOCKICC_SELCC_H__

/* target id for sparc v8 block icc + selcc is 1 */
#define TARGET_ID	0x0003

/*===============================*/
/*        Set operations         */
/*===============================*/

/* op = 1 */
#define FORMAT_1(a) {\
	(a) |= (1<<30); \
	(a) &= ~(1<<31); \
}

/* op = 0 */
#define FORMAT_2(a) {\
	(a) &= ~((1<<31)|(1<<30)); \
}

/* op = 3 */
#define FORMAT_3_mem(a) {\
	(a) |= ((1<<31)|(1<<30)); \
}

/* op = 2 */
#define FORMAT_3_oth(a) {\
	(a) &= ~(1<<30); \
	(a) |= (1<<31); \
}

/* set disp30 */
#define SET_DISP30(a, b) {\
	(a) &= ((1<<31)|(1<<30)); \
	(a) |= ((b) & 0x3FFFFFFF); \
}

/* set destination register */
#define SET_RD(a, b) {\
	(a) &= ~((0x1f) << 25); \
	(a) |= (((b) & 0x1f) << 25); \
}

/* set op2 code */
#define SET_OP2(a, b) {\
	(a) &= ~((0x7) << 22); \
	(a) |= (((b) & 0x7) << 22); \
}

/* set imm22 */
#define SET_IMM22(a, b) {\
	(a) &= ~((0x3FFFFF) << 0); \
	(a) |= (((b) & 0x3FFFFF) << 0); \
}

/* set op3 code */
#define SET_OP3(a, b) {\
	(a) &= ~((0x3f) << 19); \
	(a) |= (((b) & 0x3f) << 19); \
}

/* set rs1 */
#define SET_RS1(a, b) {\
	(a) &= ~((0x1f) << 14); \
	(a) |= (((b) & 0x1f) << 14); \
}

/* set rs2 */
#define SET_RS2(a, b) {\
	(a) &= ~((0x1f) << 0); \
	(a) |= (((b) & 0x1f) << 0); \
	(a) &= ~(1<<13); \
}
	
/* set simm13 */
#define SET_SIMM13(a, b) {\
	(a) &= ~((0x1fff) << 0); \
	(a) |= (((b) & 0x1fff) << 0); \
	(a) |= (1<<13); \
}

/* set asi */
#define SET_ASI(a, b) {\
	(a) &= ~((0xff) << 5); \
	(a) |= (((b) & 0xff) << 5); \
}

/* set condition code */
#define SET_CC(a, b) {\
	(a) &= ~((0xf) << 25); \
	(a) |= (((b) & 0xf) << 25); \
}

/* set a bit */
#define SET_A(a, b) {\
	(a) &= ~(1<<29); \
	(a) |= (((b) & 0x1) << 29); \
}

/*===============================*/
/*        Get operations         */
/*===============================*/

/* get op */ 
#define GET_OP(a)	(((a) >> 30) & 0x3)

/* get op2 */
#define GET_OP2(a)	(((a) >> 22) & 0x7)

/* get op3 */
#define GET_OP3(a)	(((a) >> 19) & 0x3f)

/* get disp30 */
#define GET_DISP30(a)	((a) & 0x3FFFFFFF)

/* get destination register */
#define GET_RD(a)	(((a) >> 25) & 0x1f)

/* get imm22 */
#define GET_IMM22(a)	((a) & 0x3FFFFF)

/* get source1 register */
#define GET_RS1(a)		(((a) >> 14) & 0x1f)

/* get source2 register */
#define GET_RS2(a)		((a) & 0x1f)

/* get asi */
#define GET_ASI(a)		(((a) >> 5) & 0xff)

/* get simm13 */
#define GET_SIMM13(a)	((a) & 0x1fff)

/* get immediate bit */
#define GET_I(a)		(((a) >> 13) & 0x1)

/* get condition code */
#define GET_CC(a)	(((a) >> 25) & 0xf)

#define GET_A(a)	(((a) >> 29) & 0x1)

/*===============================*/
/*      Opcode definitions       */
/*===============================*/

/* op2 codes for sethi and branch instructions */
#define OP2_UNIMP	0
/* new hwloop instruction */
#define OP2_HWLOOP	1
#define OP2_BICC	2
/* new conditional select instruction */
#define OP2_SELCC	3
#define OP2_SETHI	4
/* new predicated blocks on condition codes instruction */
#define OP2_PREDBLOCKSCC	5
#define OP2_FBFCC	6
#define OP2_CBCCC	7

/* op3 codes for load integer instructions */
#define OP3_LDSB	0x09
#define OP3_LDSH	0x0a
#define OP3_LDUB	0x01
#define OP3_LDUH	0x02
#define OP3_LD		0x00
#define OP3_LDD		0x03

/* op3 codes for extended load integer instructions */
#define OP3_LDSBA	0x19
#define OP3_LDSHA	0x1a
#define OP3_LDUBA	0x11
#define OP3_LDUHA	0x12
#define OP3_LDA		0x10
#define OP3_LDDA	0x13

/* op3 codes for store integer instructions */
#define OP3_STB		0x05
#define OP3_STH		0x06
#define OP3_ST		0x04
#define OP3_STD		0x07

/* op3 codes for extended store integer instructions */
#define OP3_STBA	0x15
#define OP3_STHA	0x16
#define OP3_STA		0x14
#define OP3_STDA	0x17

/* op3 codes for atomic load/store instructions */
#define OP3_LDSTUB	0x0d
#define OP3_LDSTUBA	0x1d

/* op3 codes for swap instructions */
#define OP3_SWAP	0x0f
#define OP3_SWAPA	0x1f

/* op3 codes for logical instructions */
#define OP3_AND		0x01
#define OP3_ANDCC	0x11
#define OP3_ANDN	0x05
#define OP3_ANDNCC	0x15
#define OP3_OR		0x02
#define OP3_ORCC	0x12
#define OP3_ORN		0x06
#define	OP3_ORNCC	0x16
#define OP3_XOR		0x03
#define OP3_XORCC	0x13
#define OP3_XNOR	0x07
#define OP3_XNORCC	0x17

/* op3 codes for shift instructions */
#define OP3_SLL		0x25
#define OP3_SRL		0x26
#define OP3_SRA		0x27

/* op3 codes for add instructions */
#define OP3_ADD		0x00
#define OP3_ADDCC	0x10
#define OP3_ADDX	0x08
#define OP3_ADDXCC	0x18

/* op3 codes for tagged add instructions */
#define OP3_TADDCC	0x20
#define OP3_TADDCCTV	0x22

/* op3 codes for subtract instructions */
#define OP3_SUB		0x04
#define OP3_SUBCC	0x14
#define OP3_SUBX	0x0c
#define OP3_SUBXCC	0x1c

/* op3 codes for tagged subtract instructions */
#define OP3_TSUBCC	0x21
#define OP3_TSUBCCTV	0x23

/* op3 codes for multiply step instruction */
#define OP3_MULSCC	0x24

/* op3 codes for multiply instructions */
#define OP3_UMUL	0x0a
#define OP3_SMUL	0x0b
#define OP3_UMULCC	0x1a
#define OP3_SMULCC	0x1b

/* op3 codes for divide instructions */
#define OP3_UDIV	0x0e
#define OP3_SDIV	0x0f
#define OP3_UDIVCC	0x1e
#define	OP3_SDIVCC	0x1f

/* op3 codes for save and restore instructions */
#define OP3_SAVE	0x3c
#define OP3_RESTORE	0x3d

/* op3 codes for jump and link instructions */
#define OP3_JUMPL	0x38

/* op3 codes for read Y instruction */
#define OP3_RDY		0x28

/* op3 codes for write Y instruction */
#define OP3_WRY		0x30

/* take the UNIMP opcode for SIMCYCLES */
#define OP2_SIMCYCLES	OP2_UNIMP
#define SIM_CYCLES_PRINT	0
#define SIM_CYCLES_CLEAR	1

/*===============================*/
/*      HWLoop definitions       */
/*===============================*/

/* get rd-bits to determine hwloop type */
#define GET_HWLOOP_TYPE(a)	(((a) >> 25) & 0x1f)

/* set rd-bits to determine hwloop type */
#define SET_HWLOOP_TYPE(a, b) {\
	(a) &= ~((0x1f) << 25); \
	(a) |= (((b) & 0x1f) << 25); \
}

#define HWLOOP_TYPE_SET_S	0
#define HWLOOP_TYPE_SET_E	1
#define HWLOOP_TYPE_SET_B_IMM	2
#define HWLOOP_TYPE_SET_B_REG	3
#define HWLOOP_TYPE_START	4

/*===============================*/
/*       SELcc definitions       */
/*===============================*/

#define SELCC_TYPE_REG_REG	0
#define SELCC_TYPE_REG_IMM	1
#define SELCC_TYPE_IMM_IMM	2

/* set type for selcc instruction */
#define SELCC_SET_TYPE(a,b) {\
	(a) &= ~((0x3) << 20); \
	(a) |= (((b) & 0x3) << 20); \
}

/* get type of selcc instruction */
#define SELCC_GET_TYPE(a)	(((a) >> 20) & 0x3)

/* set integer condition codes for selcc instruction */
#define SELCC_SET_ICC(a,b) {\
	(a) &= ~((0xf) << 16); \
	(a) |= (((b) & 0xf) << 16); \
}

/* get integer condition codes of selcc instruction */
#define SELCC_GET_ICC(a)	(((a) >> 16) & 0xf)

/* set source1 register for selcc instruction */
#define SELCC_SET_RS1(a,b) {\
	(a) &= ~((0x1f) << 11); \
	(a) |= (((b) & 0x1f) << 11); \
}

/* we have to use a separate set function because we do not want
   bit 13 to be set implicitly */
#define SELCC_SET_RS2(a, b) {\
	(a) &= ~((0x1f) << 0); \
	(a) |= (((b) & 0x1f) << 0); \
}

/* note: use GET_RS2() for source2 */

/* get source1 register of selcc instruction */
#define SELCC_GET_RS1(a)	(((a) >> 11) & 0x1f)

/* set signed immediate with 11 bits */
#define SELCC_SET_SIMM11(a,b) {\
	(a) &= ~((0x7ff) << 0); \
	(a) |= (((b) & 0x7ff) << 0); \
}

/* get signed immediate with 11 bits */
#define SELCC_GET_SIMM11(a)	(((a) >> 0) & 0x7ff)

/* set signed src1 immediate with 8 bits */
#define SELCC_SET_SRC1_IMM8(a,b) {\
	(a) &= ~((0xff) << 8); \
	(a) |= (((b) & 0xff) << 8);\
}

/* get signed src1 immediate with 8 bits */
#define SELCC_GET_SRC1_IMM8(a)	(((a) >> 8) & 0xff)

/* set signed src2 immediate with 8 bits */
#define SELCC_SET_SRC2_IMM8(a,b) {\
	(a) &= ~((0xff) << 0); \
	(a) |= (((b) & 0xff) << 0);\
}

/* get signed src2 immediate with 8 bits */
#define SELCC_GET_SRC2_IMM8(a)	(((a) >> 0) & 0xff)

/*===============================*/
/*    Predblocks definitions     */
/*===============================*/

/* set A-bit to set predbegin */
#define PRED_BLOCK_SET_BEGIN(a) {\
	(a) |= (1<<29); \
}

/* clear A-bit to set predend */
#define PRED_BLOCK_SET_END(a) {\
	(a) &= ~(1<<29); \
}

/* get A-bit to determine begin or end of block */
#define PRED_BLOCK_IS_BEGIN(a) (((a) >> 29) & 0x1)


#endif /* __SPARC_V8_BLOCKICC_SELCC_H__ */
