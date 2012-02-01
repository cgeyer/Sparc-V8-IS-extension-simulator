/*
 * SPARC V8 Instruction Set Extension Simulator
 *
 * File: include/gen_simulator.h
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

#ifndef __GEN_SIMULATOR_H__
#define __GEN_SIMULATOR_H__

#include <stdint.h>
#include <stdio.h>

#include "sparc_target.h"

typedef struct {
	uint16_t		target_id;
	uint32_t		memory_size;
	uint32_t		instruction_size;
} simulator_header_t;

typedef void (* void_fct_t)(void);
typedef int (* boolean_fct_t)(void);
typedef uint32_t (* size_fct_t)(void);
typedef void (* read_file_fct_t)(FILE*);
typedef void (* write_file_fct_t)(FILE*);
typedef simulator_header_t* (* file_hdr_fct_t)(void);
typedef sparc_instruction** (* get_paddr_fct_t)(void);
typedef int (* sim_fct_t)(FILE*);

typedef void (* error_fct_t)(char*);

typedef struct {
	read_file_fct_t			readFileHeader;
	file_hdr_fct_t			getFileHeader;
	boolean_fct_t			checkTargetID;
	read_file_fct_t			readMemory;
	read_file_fct_t			readInstructions;
	write_file_fct_t		printInstructions;
	write_file_fct_t		printMemory;
	write_file_fct_t		printRegisters;
	write_file_fct_t		printResults;
	sim_fct_t				simulateStep;
	void_fct_t				resetSimulator;
	get_paddr_fct_t			getInstructions;
	size_fct_t				getNumberOfInstructions;
	void_fct_t				cleanUp;
} gen_simulator_t;

typedef int (* simulator_init_fct_t)(gen_simulator_t* simulator, error_fct_t);

int gen_simulator_init(gen_simulator_t* simulator, error_fct_t);

#endif /* __GEN_SIMULATOR_H__ */
