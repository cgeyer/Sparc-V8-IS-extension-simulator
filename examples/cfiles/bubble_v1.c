/*
 * SPARC V8 Instruction Set Extension Simulator
 *                                                                               
 * File: examples/cfiles/bubble_v1.c
 *                                                                               
 * Copyright (c) 2012 Clemens Bernhard Geyer <clemens.geyer@gmail.com>
 *
 * Copyright (c) original implementation of Bubble Sort:
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

#define SIZE 100
#define TEST_CASES 3

static void BubbleSort(int a[])
{
	int i;
	int j;
	int t;
	for(i=SIZE-1; i>0; i--)
	{
		for(j=1; j<=i; j++) /* max. iterations: SIZE-1 */
		{
			/* execution frequency: SIZE*(SIZE-1)/2 */
			if (a[j-1] > a[j])
			{
				t = a[j];
				/* swap */
				a[j] = a[j-1];
				a[j-1] = t;
			}
		}
	}
}

static void Initialize(int Array[], int seed)
/*
 * Initializes given array with randomly generated integers.
 */
{
   int  Index;

   for (Index = 0; Index < SIZE; Index ++) {
      if (seed == 0) {
         Array[Index] = Index*(-1);
      } else if (seed == 1) {
         Array[Index] = Index;
      } else if (seed == 2) {
         Array[Index] = Index ^ (0xf125a672);
      }
   }
}

int main(void)
{
   int i,j;
   int Array[SIZE];
   for (i = 0; i < TEST_CASES; i++) {
      Initialize(Array, i);
      
	  asm("sim-clearcycles");
      BubbleSort(Array);  
      asm("sim-printcycles"); 

      for (j = 0; j < SIZE - 1; j++) {
         if (Array[j] > Array[j + 1]) {
            return -1;
         }
      }
   }
   
   return 0;
}

