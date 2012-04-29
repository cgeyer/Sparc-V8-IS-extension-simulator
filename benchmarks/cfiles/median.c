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
 * @brief naive median filter implementation with quicksort
 * @param[in] elem_type m: array containing data
 * @param[in] int n: contais number of data elements
 */

static elem_type median(elem_type m[], int n) {

	int start, end, lowerindex, upperindex;
	elem_type *tmp = m; /* malloc(sizeof(elem_type)*n); */
	elem_type element, tmpelement;

	start = 0; end = n - 1;

	/* copy array if legal address was returned */
	if(!tmp) {
		return ((elem_type) 0);
	}
	/* implement quicksort to find median element */
	/* # ai: loop here MAX((@n+1)/2); */
	while(1) { 

		element = tmp[start];
		lowerindex = start + 1;
		upperindex = end;
		
		/* # ai: loop here MAX((@n+1)/2); */
		do {
			while (upperindex > (start + 1) && tmp[upperindex] >= element) {
				/* ai: label here = "upper"; */
				upperindex--; 
			}

			while (lowerindex < end && tmp[lowerindex] <= element) {
				/* ai: label here = "lower"; */
				lowerindex++; 
			}
			/* ai: flow ("upper") + ("lower") <= 1089 "median"; */

			if (lowerindex < upperindex) {
				tmpelement = tmp[lowerindex];
				tmp[lowerindex] = tmp[upperindex]; 
				tmp[upperindex] = tmpelement;
			}
			
		} while (lowerindex < upperindex);

		if (tmp[upperindex] < element) {
			tmp[start] = tmp[upperindex];
			tmp[upperindex] = element;
		} else {
			upperindex = start;
		}

		if (upperindex < ((n + 1)/2 - 1)) {
			start = upperindex + 1;
		}
		else if (upperindex > ((n + 1)/2 - 1)) {
			end = upperindex - 1;
		}
		else {
			break;
		}
	
	} 

	return tmp[(n+1)/2 - 1];
}


int main() {

	int i;
	elem_type test_median;
	elem_type real_median;
	
	for (i = 0; i < TEST_DATA_LENGTH; i++) {
		/* clear cycles before test call */
		asm("sim-clearcycles");
		test_median = median(testInputVector[i], TEST_DATA_ELEMENTS);
		/* print out simulated cycles for each vector */
		asm("sim-printcycles");
		real_median = testOutputVector[i];
		if (test_median != real_median) {
			return -1;
		}
	}

	return 0;
}


