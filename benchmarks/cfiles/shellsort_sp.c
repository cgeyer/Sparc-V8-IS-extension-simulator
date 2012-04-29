/* Taken from WCET Lab */
/*
Copyright (c) <2010>, <Christopher Helpa, Alexander Oh, Dieter Steiner>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "sort.h"

static bool sort(AbstractType *data, int N)
{

 int i, j, k, h,lastj;
 bool abort,done;
 AbstractType v;
 //Input length is limited, so we don't need to use all increments
 
  /*const int incs[11] =   {8470,3850,1750,701,301,
                        132,57,23, 10, 4,
                        1};*/
 const int incs[2] =   {4,1};                        
 N=6;
 for ( k = 0;
 	k < 2;
 	k++) /* ai: loop here MAX (2); */
   for (h = incs[k], i = h;
    i <6;
     i++) 
   { /* ai: loop here MAX (6); */
     v = data[i];
     j = i;
     lastj = i;
     
     abort = (data[j-h] < v); //initial loop conditions
     done = false;
     
     for( ;
      j >= h;  
      j -= h) 
     {/* ai: loop here MAX (5); */
       if(!done && !abort) abort = (data[j-h] < v);
       if(!done && !abort) data[j] = data[j-h];
       if(!done && abort) done = true;  //are we done?
       if(done && abort) lastj = j; //on break store the last j
       if(done && abort) abort = false;  //never execute something from this loop again     
       
     }
     if(!done) lastj = j; //loop terminated withoput a break
     data[lastj] = v;
   }
   return true;
}

int main ( int length, char *argv[] )
{
    int i, j;

    for(i=0;i<720;i++)  {
		asm("sim-clearcycles");
		sort ( test_arr[i], 6 );
		asm("sim-printcycles");
		for (j=0;j<6;j++) {
			if (test_arr[i][j] != j+1) {
				return -1;
			}
		}
	}

	return 0;

}

