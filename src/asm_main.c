/*
 * SPARC V8 Instruction Set Extension Simulator
 *
 * File: src/asm_main.c
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
#include <dlfcn.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
 
#include "gen_assembler.h"
#include "debug.h"

/** Name of the current program. */
char* progname;
/** Globally available assembler struct for parser. */
gen_assembler_t* assembler;
/** Needed string for optarg() call. */
char* optarg;
/** Externally defined output file stream of yacc. */
extern FILE* yyout;
/** Externally defined input file stream of yacc. */
extern FILE* yyin;

extern int yyerror(char* e);

/**
  * @brief Prints out a usage message on the given file stream.
  * @param[in] out The file stream where to write the message.
  */
void usage(FILE* out) {
	fprintf(out, "Usage: %s -t <target> [-i <assemblerfile>] [-o <binfile>]\n", progname);
}

int main(int argc, char** argv) {

	/* declare variables for dynamic opening of shared libraries */
	void* lib_handle;
	char* dl_error;

	/* function pointer to assembler init function */
	assembler_init_fct_t init_fct;

	/* paths to all supported targets */
	const char* asm_libraries[] = { 
		"shared/libasm_sparc_v8.so",
		"shared/libasm_sparc_v8-blockicc-movcc.so",
		"shared/libasm_sparc_v8-blockpreg-selcc.so",
		"shared/libasm_sparc_v8-blockicc-selcc.so",
	};

	/* save which target is selected,
	 * currently undefined */
	int asm_lib = -1;
	/* return status of getopt() */
	int opt;

	/* make program name globally available */
	progname = argv[0];

	/* set default values for yyin and yyout */
	yyin = stdin;
	yyout = stdout;

	/* parse input options */
	while ((opt = getopt(argc, argv, "ht:i:o:")) != -1) {
		switch (opt) {
			case 't':
				if (!(strcmp(optarg, "v8"))) {
					asm_lib = 0;
				} else if (!(strcmp(optarg, "v8-blockicc-movcc"))) {
					asm_lib = 1;
				} else if (!(strcmp(optarg, "v8-blockpreg-selcc"))) {
					asm_lib = 2;
				} else if (!(strcmp(optarg, "v8-blockicc-selcc"))) {
					asm_lib = 3;
				}
				break;
			case 'i':
				yyin = fopen(optarg, "r");
				if (yyin == NULL) {
					fprintf(stderr, "%s: Could not open file \"%s\" for reading!\n", progname, optarg);
					exit(EXIT_FAILURE);
				}
				break;
			case 'o':
				yyout = fopen(optarg, "w");
				if (yyout == NULL) {
					fprintf(stderr, "%s: Could not open file \"%s\" for writing!\n", progname, optarg);
					exit(EXIT_FAILURE);
				}
				break;
			case 'h':
				usage(stdout);
				break;
			default:
				fprintf(stderr, "%s: Unknown option \"-%c\".\n", progname, opt);
				exit(EXIT_FAILURE);
		}
	}

	/* check, whether valid target has been specified */
	if (asm_lib == -1) {
		fprintf(stderr, "%s: No valid target has been specified! Possible targets are:\n"
			"\tv8                 - Original Sparc-V8 target\n" 
			"\tv8-blockicc-movcc  - Original Sparc-V8 target with conditional moves, "
			"predicated blocks on condition codes and hardware loops.\n"
			"\tv8-blockpreg-selcc - Original Sparc-V8 target with conditional select, "
			"predicated blocks on predicate registers and hardware loops.\n" 
			"\tv8-blockicc-selcc  - Original Sparc-V8 target with conditional select, "
			"predicated blocks on condition codes and hardware loops.\n\n", 
			progname);
		exit(EXIT_FAILURE);
	}

	/* allocate memory for assembler struct */
	assembler = malloc(sizeof(gen_assembler_t));
	if (!assembler) {
		fprintf(stderr, "%s: Could not allocate memory for generic assembler!\n",
			progname);
		exit(EXIT_FAILURE);
	}

	/* open shared library */
	lib_handle = dlopen(asm_libraries[asm_lib], RTLD_LAZY);
    if (!lib_handle) 
    {   
        fprintf(stderr, "%s\n", dlerror());
		exit(EXIT_FAILURE);
    }   
    
	/* get assembler init function */
    *(void **) (&init_fct) = dlsym(lib_handle, "assembler_init");
    if ((dl_error = dlerror()) != NULL)  
    {   
        dlclose(lib_handle);
        fprintf(stderr, "%s: %s\n", progname, dl_error);
		exit(EXIT_FAILURE);
    }

	/* let assembler register its target specific functions */
	if (init_fct(assembler)) {
		fprintf(stderr, "%s: Could not initialize target specific assembler correctly!\n", 
			progname);
	}

	/* let assembler register its target specific functions */
	if (gen_assembler_init(assembler)) {
		fprintf(stderr, "%s: Could not initialize generic assembler correctly!\n", 
			progname);
	}

	/* start parsing code */
	if (yyparse()) {
		fprintf(stderr, "Terminating assembler...\n");
		exit(EXIT_FAILURE);
	}

	/* check Labels for all instructions */
	assembler->checkLabels();

	/* print all data */
	assembler->printData(yyout);

	/* print all instructions */
	assembler->printInstructions(yyout);

	/* perform clean up */
	assembler->cleanUp();

	/* free memory of assembler data structure */
	free(assembler);
	
	/* close all open file handles */
	if (yyin != stdin) {
		fclose(yyin);
	}
	if (yyout != stdout) {
		fclose(yyout);
	}

	/* close library handle */
	if (dlclose(lib_handle)) {
		fprintf(stderr, "%s: Could not close shared library!",
			progname);
		exit(EXIT_FAILURE);
	}
	
	exit(EXIT_SUCCESS);

}
