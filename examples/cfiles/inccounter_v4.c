/*
 * SPARC V8 Instruction Set Extension Simulator
 *                                                                               
 * File: examples/cfiles/iccounter_v4.c
 *                                                                               
 * Copyright (c) 2012 Clemens Bernhard Geyer <clemens.geyer@gmail.com>
 *
 * Copyright (c) original implementation of Increment Counter:
 * Peter Puschner, "Evaluation of the Single-Path Approach and 
 * WCET-Oriented Programming", Technical report, TU Vienna, 2007.
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
 *
 */

#define COUNTERSIZE 10
typedef unsigned char* COUNTER;

static void inc_counter(COUNTER counter)
{
	int idx, inc_val;
	inc_val = 1;
	for(idx=0; idx<COUNTERSIZE; idx++)
	{
		unsigned char tmp;
		tmp = counter[idx];
		counter[idx] = tmp + inc_val;
		if (counter[idx] > 0)
			inc_val = 0;
	}
}

static void InitializeCounter(COUNTER counter, unsigned char seed) {
	int idx;
	for (idx = 0; idx < COUNTERSIZE - 1; idx++) {
		counter[idx] = seed;
	}
}

int main(void) {
	unsigned char counter[COUNTERSIZE];
	int i;
	InitializeCounter(counter, 0xff);
	for (i = 0; i < 0xffff; i++) {
		asm("sim-clearcycles");
		inc_counter(counter);
		asm("sim-printcycles");
	}
	return 0;
}
