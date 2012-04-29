/* taken from WCET lab */

#include "sort.h"


static bool sort(AbstractType *data, int N)
{
  // using shellsort with Ciura Sequence
 // http://en.wikipedia.org/wiki/Shell_sort
 // http://sun.aei.polsl.pl/~mciura/publikacje/shellsort.pdf
 int i, j, k, h;
 AbstractType v;
 //Input length is limited, so we don't need to use all increments
 
 /*const int incs[11] =   {8470,3850,1750,701,301,
                        132,57,23, 10, 4,
                        1};*/
 const int incs[2] =   {4,1};                        
 for ( k = 0; k < 2; k++)/*ai: loop here MAX 2*/
   for (h = incs[k], i = h; i <N; i++)/*ai: loop here MAX 6 */
   {
     v = data[i];
     j = i;
     while ((j >= h) && data[j-h] > v) /*ai: loop here MAX 5 */
     {
       data[j] = data[j-h];
       j -= h;
     }
     data[j] = v;
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

