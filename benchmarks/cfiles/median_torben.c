/* taken from WCET lab */
/*

 Copyright (c) 2010, Clemens Bernhard Geyer, TU Vienna.
 All rights reserved.
 
 Redistribution and use in source and binary forms, 
 with or without modification, are permitted 
 provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, 
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, 
      this list of conditions and the following disclaimer in the documentation 
      and/or other materials provided with the distribution.
    * Neither the name of the TU Vienna nor the names of its contributors may be
      used to endorse or promote products derived from this software without 
      specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
 PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS 
 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, 
 OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
 OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
 OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, 
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "median.h"
#include "testdata.h"


/*
 * @brief fast implementation of median filter
 * @param[in] elem_type m: array containing data
 * @param[in] int n: contais number of data elements
 */
static elem_type torben(elem_type m[], int n) {
	
	int i, less, greater, equal;
	elem_type min, max, guess, maxltguess, mingtguess;
	min = max = m[0];

	/* ai: loop here MAX(@n); */
	for (i=1 ; i<n ; i++) { 	
		if (m[i]<min) min=m[i]; /* ai: flow (here) <= 64 "torben"; */
		if (m[i]>max) max=m[i]; /* ai: flow (here) <= 64 "torben"; */
	}

	/* ai: loop here MAX((@n+1)/2); */
	while (1) {
		guess = (min+max)/2;
		less = 0; greater = 0; equal = 0;
		maxltguess = min;
		mingtguess = max;
		
		/* ai: loop here MAX(@n); */
		for (i=0; i<n; i++) { 
			if (m[i]<guess) {
				less++; 
				if (m[i]>maxltguess) maxltguess = m[i]; /* ai: flow (here) <= 448 "torben"; */
			} else if (m[i]>guess) {
				greater++; /* # max n-1 times for one iteration */
				if (m[i]<mingtguess) mingtguess = m[i]; /* ai: flow (here) <= 448 "torben"; */
			} else equal++; /* ai: flow (here) <= 65 "torben"; */ /* # max n-1 times for one iteration */
		}

		if (less <= (n+1)/2 && greater <= (n+1)/2) break;
		else if (less>greater) max = maxltguess;
		else min = mingtguess;

	}

	if (less >= (n+1)/2) return maxltguess;
	else if (less+equal >= (n+1)/2) return guess;
	else return mingtguess;
}


int main() {

	int i;
	elem_type test_median;
	elem_type real_median;
	
	for (i = 0; i < TEST_DATA_LENGTH; i++) {
		/* clear cycles before test call */
		asm("sim-clearcycles");
		test_median = torben(testInputVector[i], TEST_DATA_ELEMENTS);
		/* print out simulated cycles for each vector */
		asm("sim-printcycles");
		real_median = testOutputVector[i];
		if (test_median != real_median) {
			return -1;
		}
	}

	return 0;
}


