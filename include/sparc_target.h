/*
 * SPARC V8 Instruction Set Extension Simulator
 *
 * File: include/sparc_target.h
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

#ifndef __SPARC_TARGET_H__
#define __SPARC_TARGET_H__

#include <stdint.h>

/* register number definitions */
#define G_REGISTER 0
#define O_REGISTER 8
#define L_REGISTER 16
#define I_REGISTER 24
/* #define P_REGISTER 32 */

/* special register numbers */
#define RET_VAL_REGISTER 8
#define SP_REGISTER 14
#define CALL_ADDR_REGISTER 15
#define FP_REGISTER 30
#define Y_REGISTER_NO 0

/** define number of windows - maximum 32 */
#define NWINDOWS 16
/* define WIM_MASK depending on NWINDOWS */
#if NWINDOWS > 32 || NWINDOWS < 2
	#error "NWINDOS must be in [2,32] "
#endif
#if NWINDOWS == 32
	#define WIM_MASK 0x0
#else
	#define WIM_MASK ~((1<<NWINDOWS) - 1)
#endif /* NWINDOWS == 32 */

/** define initialization mask for psr */
#define PSR_INIT_MASK	((0<<13)|(0<<12)|(0<<7)|((NWINDOWS-1)<<0)) 

/* define macros for reading and writing CWP of psr */
#define PSR_GET_CWP(a)	((a) & 0x1f)
#define PSR_SET_CWP(a,b) {\
	(a) &= ~(0x1f); \
	(a) |= ((b) & (0x1f)); \
}
#define PSR_INC_CWP(a) {\
	register int tmp = PSR_GET_CWP(a); \
	tmp++; \
	PSR_SET_CWP(a, tmp); \
}
#define PSR_DEC_CWP(a) {\
	register int tmp = PSR_GET_CWP(a); \
	tmp--; \
	PSR_SET_CWP(a, tmp); \
}

/* define macros for reading out iccs of psr */
#define PSR_GET_C(a)	(((a) >> 20) & 0x1)
#define PSR_GET_V(a)	(((a) >> 21) & 0x1)
#define PSR_GET_Z(a)	(((a) >> 22) & 0x1)
#define PSR_GET_N(a)	(((a) >> 23) & 0x1)

/* define macros for setting iccs of psr */
#define PSR_SET_C(a) {\
	(a) |= (1 << 20); \
}
#define PSR_SET_V(a) {\
	(a) |= (1 << 21); \
}
#define PSR_SET_Z(a) {\
	(a) |= (1 << 22); \
}
#define PSR_SET_N(a) {\
	(a) |= (1 << 23); \
}

/* define macros for clearing iccs of psr */
#define PSR_CLR_C(a) {\
	(a) &= ~(1 << 20); \
}
#define PSR_CLR_V(a) {\
	(a) &= ~(1 << 21); \
}
#define PSR_CLR_Z(a) {\
	(a) &= ~(1 << 22); \
}
#define PSR_CLR_N(a) {\
	(a) &= ~(1 << 23); \
}
#define PSR_CLR_ICCS(a) {\
	(a) &= ~((0x1f) << 20); \
}
/*
#define PSR_SET_ICCS(a,b) {\
	(a) &= ~((0x1f) << 20); \
	(a) |= (((b) & (0x1f)) << 20); \
}
*/

/** define free data memory size in bytes */
#define FREE_MEMORY_SIZE	512

/* define cycle times for various instructison,
   refer to Sparc V8 manual, p.295 ff */
#define CYCLES_INTEGER_INSTR 		1
#define CYCLES_LOAD_SINGLE 			2
#define CYCLES_LOAD_DOUBLE			3
#define CYCLES_STORE_SINGLE			3
#define CYCLES_STORE_DOUBLE			4
#define CYLCES_SWAP					4
#define CYCLES_LDSTUB				4
#define CYCLES_MUL					5
/* not defined in manual, but assumed */
#define CYCLES_DIV					5

/** standard return address of main function */
#define END_OF_INS_MEM	0xffffffff


/* condition codes for the sparc target */
typedef enum {
	CC_A = 8,
	CC_N = 0,
	CC_NE = 9,
	CC_E = 1,
	CC_G = 10,
	CC_LE = 2,
	CC_GE = 11,
	CC_L = 3,
	CC_GU = 12,
	CC_LEU = 4,
	CC_CC = 13,
	CC_CS = 5,
	CC_POS = 14,
	CC_NEG = 6,
	CC_VC = 15,
	CC_VS = 7
} condition_codes;

typedef enum {
	OPERAND_TYPE_REGISTER = 0,
	OPERAND_TYPE_SIMM13,
	OPERAND_TYPE_SIMM11,
	OPERAND_TYPE_SIMM8,
	OPERAND_TYPE_IMM22,
	OPERAND_TYPE_LABEL,
	OPERAND_TYPE_HI_LABEL,
	OPERAND_TYPE_LOW_LABEL,
	OPERAND_TYPE_LABEL_ADDRESS,
	OPERAND_TYPE_LOOP_REG,
	OPERAND_TYPE_ICC,
	OPERAND_TYPE_PREG,
	OPERAND_TYPE_TF
} sparc_operand_type;

typedef union {
	int			reg;
	int			simm13;
	int			simm11;
	int			simm8;
	int			imm22;
	char*		label;
	unsigned	labeladdress;
	int			loopreg;
	int			icc;
	int			preg;
	int			tf;
} sparc_operand_value;

typedef struct {
	sparc_operand_type	type;
	sparc_operand_value	value;
} sparc_operand;

typedef struct {
	sparc_operand	operand1;
	sparc_operand	operand2;
} sparc_address;

typedef struct {
	int 			opcode;
	unsigned		instr_no;
	unsigned 		num_operands;
	sparc_operand*	operands;
} sparc_instruction;

struct sparc_instruction_node {
	sparc_instruction				instruction;
	struct sparc_instruction_node*	next_instruction;
};

typedef struct sparc_instruction_node sparc_instruction_node_t;

struct sparc_data_node {
	uint32_t				value;
	char*					label;
	unsigned				no_bytes;
	unsigned				data_no;
	struct sparc_data_node*	next_data;	
};

typedef struct sparc_data_node sparc_data_node_t;

struct label_node {
	char*					label_name;
	unsigned				address;
	struct label_node*		next_label;
};

typedef struct label_node label_node_t;

typedef enum {
	HWLOOP_STATE_IDLE,
	HWLOOP_STATE_ACTIVE
} hwloop_state_t;

typedef struct {
	hwloop_state_t			hwloop_state;
	uint32_t				start_address;
	uint32_t				end_address;
	uint32_t				loop_counter;
} hwloop_processor_state_t;

typedef enum {
	PREDICATE_STATE_NONE,
	PREDICATE_STATE_ICC,
	PREDICATE_STATE_PREG
} predicate_state_t;

typedef union {
	int icc;
	struct {
		int preg;
		int tf;
	} preg_condition;
} predicate_condition_t;

typedef struct {
	predicate_state_t		predicate_state;
	predicate_condition_t	predicate_condition;
} predicate_processor_state_t;

/* check whether signed immediate only has 13 bits */
#define IS_SIMM13(a) (((a) >= -4096) && ((a) < 4096))

#define TO_SIMM13(a) ((a) & 0x1FFF)

#define IS_SIMM11(a) (((a) >= -1024) && ((a) < 1024))

#define IS_SIMM8(a) (((a) >= -128) && ((a) < 128))

#define IS_UIMM22(a) (((a) >= 0) && ((a) < (1<<22)))

#define IS_IMM22(a) (((a) >= -2097152) && ((a) < 2097152))

#endif /* __SPARC_TARGET_H__ */
