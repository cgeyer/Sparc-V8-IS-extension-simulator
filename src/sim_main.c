/*
 * SPARC V8 Instruction Set Extension Simulator
 *
 * File: src/sim_main.c
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

#include "gen_simulator.h"


/** Name of the current program. */
static char* progname;
/** Needed string for optarg() call. */
char* optarg;

/* set default values for instream and outstream */
static FILE* instream; 
static FILE* outstream;

/* declare library handle for dynamic opening of shared libraries */
static void* lib_handle = 0;

/**
  * @brief Prints out a usage message on the given file stream.
  * @param[in] out The file stream where to write the message.
  */
void usage(FILE* out) {
	fprintf(out, "Usage: %s -t <target> [-i <binfile>] [-o <logfile>] [-s]\n\t-s\tTurn on silent mode.\n\n", progname);
}

/**
  * @brief Prints out an error message, closes open file handles
  * and terminates program.
  * @param[in] e Error message to be printed.
  */
void simerror(char* e) {

	fprintf(stderr, "%s: %s\n", progname, e);

	/* close all open file handles */
	if (instream != stdin) {
		fclose(instream);
	}
	if (outstream != stdout) {
		fclose(outstream);
	}
	
	if (lib_handle && dlclose(lib_handle)) {
		fprintf(stderr, "%s: Could not close shared library!",
			progname);
		exit(EXIT_FAILURE);
	}

	exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {

	/* declare error string for dynamic opening of shared libraries */
	char* dl_error;

	/* function pointer to assembler init function */
	simulator_init_fct_t init_fct;

	/* pointer to simulator object */
	gen_simulator_t* simulator;

	/* paths to all supported targets */
	const char* sim_libraries[] = { 
		"shared/libsim_sparc_v8.so",
		"shared/libsim_sparc_v8-blockicc-movcc.so",
		"shared/libsim_sparc_v8-blockpreg-selcc.so",
		"shared/libsim_sparc_v8-blockicc-selcc.so"
	};

	/* save which target is selected,
	 * currently undefined */
	int sim_lib = -1;
	/* return status of getopt() */
	int opt;

	/* saving silent status of simulator, default = not silent */
	int silent = 0;
/* 	int i; */

	/* make program name globally available */
	progname = argv[0];

	/* initialize filestreams */
	instream = stdin;
	outstream = stdout;

	/* parse input options */
	while ((opt = getopt(argc, argv, "ht:i:o:s")) != -1) {
		switch (opt) {
			case 't':
				if (!(strcmp(optarg, "v8"))) {
					sim_lib = 0;
				} else if (!(strcmp(optarg, "v8-blockicc-movcc"))) {
					sim_lib = 1;
				} else if (!(strcmp(optarg, "v8-blockpreg-selcc"))) {
					sim_lib = 2;
				} else if (!(strcmp(optarg, "v8-blockicc-selcc"))) {
					sim_lib = 3;
				}
				break;
			case 'i':
				instream = fopen(optarg, "r");
				if (instream == NULL) {
					fprintf(stderr, "%s: Could not open file \"%s\" for reading!\n", progname, optarg);
					exit(EXIT_FAILURE);
				}
				break;
			case 'o':
				outstream = fopen(optarg, "w");
				if (outstream == NULL) {
					fprintf(stderr, "%s: Could not open file \"%s\" for writing!\n", progname, optarg);
					exit(EXIT_FAILURE);
				}
				break;
			case 'h':
				usage(stdout);
				break;
			case 's':
				silent = 1;
				break;
			default:
				fprintf(stderr, "%s: Unknown option \"-%c\".\n", progname, opt);
				exit(EXIT_FAILURE);
		}
	}

	/* check, whether valid target has been specified */
	if (sim_lib == -1) {
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
	simulator = malloc(sizeof(gen_simulator_t));
	if (!simulator) {
		simerror("%s: Could not allocate memory for generic simulator!");
	}

	/* open shared library */
	lib_handle = dlopen(sim_libraries[sim_lib], RTLD_LAZY);
    if (!lib_handle) 
    {  
		free(simulator); 
        fprintf(stderr, "%s\n", dlerror());
		exit(EXIT_FAILURE);
    }   
    
	/* get assembler init function */
    *(void **) (&init_fct) = dlsym(lib_handle, "simulator_init");
    if ((dl_error = dlerror()) != NULL)  
    {   
        dlclose(lib_handle);
		free(simulator);
        fprintf(stderr, "%s: %s\n", progname, dl_error);
        fprintf(stderr, "%s: Error when opening shared library %s!\n", progname, sim_libraries[sim_lib]);
		exit(EXIT_FAILURE);
    }

	/* let assembler register its target specific functions */
	if (init_fct(simulator, simerror)) {
		free(simulator);
		simerror("Could not initialize target specific simulator correctly!");
	}

	/* let assembler register its target specific functions */
	if (gen_simulator_init(simulator, simerror)) {
		free(simulator);
		simerror("Could not initialize generic simulator correctly!");
	}

	/* read file header */
	simulator->readFileHeader(instream);

	/* check target ID */
	if ((simulator->checkTargetID())) {
		simulator->cleanUp();
		free(simulator);
		simerror("Target ID not supported by current simulator!");
	}

	/* read memory */
	simulator->readMemory(instream);

	/* read instructions */
	simulator->readInstructions(instream);

	/* initialize all registers */
	simulator->resetSimulator();

	/* print out instructions */
	simulator->printInstructions(outstream);

	/* print out memory contents */
	if (!silent) {
		simulator->printMemory(outstream);
	}

	/* print out register contents */
	/* simulator->printRegisters(outstream);*/

	/* simulate steps as long as possible */
	while(simulator->simulateStep(outstream));

	fprintf(outstream, "\nFinished simulation...\n");

	/* print out register contents */
	if (!silent) {
		simulator->printRegisters(outstream);
	}

	/* print out memory contents */
	if (!silent) {
		simulator->printMemory(outstream);
	}

	/* print results of simulation */
	simulator->printResults(outstream);

	/* clean up memory */
	simulator->cleanUp();

	/* free memory of assembler data structure */
	free(simulator);
	
	/* close all open file handles */
	if (instream != stdin) {
		fclose(instream);
	}
	if (outstream != stdout) {
		fclose(outstream);
	}

	/* close library handle */
	if (dlclose(lib_handle)) {
		fprintf(stderr, "%s: Could not close shared library!",
			progname);
		exit(EXIT_FAILURE);
	}
	
	exit(EXIT_SUCCESS);

}
