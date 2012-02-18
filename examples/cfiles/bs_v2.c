/*
 * SPARC V8 Instruction Set Extension Simulator
 *                                                                               
 * File: examples/cfiles/bs_v2.c
 *                                                                               
 * Copyright (c) 2012 Clemens Bernhard Geyer <clemens.geyer@gmail.com>
 *
 * Copyright (c) original implementation of Binary Search:
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

#define SIZE 16

int values[SIZE] = {
	1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 
	1024, 2048, 4096, 8192, 16384, 32768
};

static int binSearch(int key, int a[])
{
	int left = 0, right = SIZE - 1, idx, inc, found = 0;
	do
	{
		idx = (right + left) >> 1;
		if (a[idx] == key)
		{
			found = 1;
		}
		else if (a[idx] < key)
		{
			left = idx+1;
		}
		else
		{
			right = idx-1;
		}
	} while (!found && (right >= left));
	if (found)
	{
		return idx;
	}
	else
	{
		return -1;
	}
}

int main(void) {
	int i, key, pos;
	key = 1;
	for (i = 0; i <= SIZE; i++) {
		asm("sim-clearcycles");
		pos = binSearch(key, values);
		asm("sim-printcycles");
		if (i != SIZE) {
			if (pos != i) {
				return -1;
			}
		} else if (pos != -1) {
			return -1;
		}
		key <<= 1;
	}
	return 0;
}
