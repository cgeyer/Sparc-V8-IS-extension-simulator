/*
 * SPARC V8 Instruction Set Extension Simulator
 *                                                                               
 * File: examples/cfiles/findfirst_v1.c
 *                                                                               
 * Copyright (c) 2012 Clemens Bernhard Geyer <clemens.geyer@gmail.com>
 *
 * Copyright (c) original implementation of Find First:
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

#define SETS 25
#define SIZE 10
#define KEY 3

/* input data sets */
int numbers[SETS][SIZE] = {
	/* key occurs once, positions 0 .. SIZE-1 */
	{3, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 3, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 1, 3, 1, 1, 1, 1, 1, 1, 1},
	{1, 1, 1, 3, 1, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 3, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 3, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 1, 3, 1, 1, 1},
	{1, 1, 1, 1, 1, 1, 1, 3, 1, 1},
	{1, 1, 1, 1, 1, 1, 1, 1, 3, 1},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 3},
	/* key 	occurs twice */
	{3, 3, 1, 1, 1, 1, 1, 1, 1, 1},
	{3, 1, 3, 1, 1, 1, 1, 1, 1, 1},
	{3, 1, 1, 3, 1, 1, 1, 1, 1, 1},
	{3, 1, 1, 1, 1, 1, 1, 1, 1, 3},
	{1, 3, 1, 1, 1, 1, 1, 1, 1, 3},
	{1, 1, 3, 1, 1, 1, 1, 1, 1, 3},
	{1, 1, 1, 3, 1, 1, 1, 1, 1, 3},
	/* key occurs three times */
	{3, 3, 3, 1, 1, 1, 1, 1, 1, 1},
	{3, 3, 1, 3, 1, 1, 1, 1, 1, 1},
	{3, 3, 1, 1, 1, 1, 1, 1, 1, 3},
	{3, 1, 3, 1, 1, 1, 1, 1, 1, 3},
	{3, 1, 1, 1, 1, 1, 1, 3, 1, 3},
	{3, 1, 1, 1, 1, 1, 1, 1, 3, 3},
	/* all elements equal key */
	{3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
	/* no occurrence of key */
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};

static int answers[SETS] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
	0, 0, 0, 0, 1, 2, 3,
	0, 0, 0, 0, 0, 0,
	0,
	SIZE
};


static int findfirst(int key, int a[])
{
	int i;
	int position = SIZE;
	for(i=0; i<=SIZE-1; i++) /* max. iterations: SIZE */
	{
		if (a[i] == key)
		{
			position = i;
			break;
		}
	}
	return position;
}

int main(void) {
	int i, pos;

	for (i = 0; i < SETS; i++) {

		asm("sim-clearcycles");
		pos = findfirst(KEY, numbers[i]);
		asm("sim-printcycles");

		if (pos != answers[i]) {
			return -1;
		}

	}
	return 0;
}

