/* taken from http://www.mrtc.mdh.se/projects/wcet/benchmarks.html */

/* bsort100.c */

/* All output disabled for wcsim */
#define WCSIM 1

/* A read from this address will result in an known value of 1 */
/* #define KNOWN_VALUE (int)(*((char *)0x80200001)) */
#define KNOWN_VALUE 1

/* A read from this address will result in an unknown value */
#define UNKNOWN_VALUE (int)(*((char *)0x80200003))


#include <sys/types.h>
#include <sys/times.h>
#include <stdio.h>

#define WORSTCASE 1
#define FALSE 0
#define TRUE 1
#define NUMELEMS 100
#define MAXDIM   (NUMELEMS+1)

/* BUBBLESORT BENCHMARK PROGRAM:
 * This program tests the basic loop constructs, integer comparisons,
 * and simple array handling of compilers by sorting 10 arrays of
 * randomly generated integers.
 */



static void Initialize(int Array[])
/*
 * Initializes given array with randomly generated integers.
 */
{
   int  Index, fact;

#ifdef WORSTCASE
   fact = -1;
#else
   fact = 1;
#endif

   for (Index = 1; Index <= NUMELEMS; Index ++)
      Array[Index] = Index*fact * KNOWN_VALUE;
}

static void BubbleSort(int Array[])
/*
 * Sorts an array of integers of size NUMELEMS in ascending order.
 */
{
   int Sorted = FALSE;
   int Temp, LastIndex, Index, i;

   for (i = 1;
	i <= NUMELEMS-1;           /* apsim_loop 1 0 */
	i++)
   {
      Sorted = TRUE;
      for (Index = 1;
	   Index <= NUMELEMS-1;      /* apsim_loop 10 1 */
	   Index ++) {
         if (Index > NUMELEMS-i)
            break;
         if (Array[Index] > Array[Index + 1])
         {
            Temp = Array[Index];
            Array[Index] = Array[Index+1];
            Array[Index+1] = Temp;
            Sorted = FALSE;
         }
      }

      if (Sorted)
         break;
   }
}

int main(void)
{
   int i;
   int Array[MAXDIM];
   Initialize(Array);
   asm("sim-clearcycles");
   BubbleSort(Array);  
   asm("sim-printcycles"); 
   for (i = 1; i <= NUMELEMS - 1; i++) {
      if (Array[i] > Array[i + 1]) {
         return -1;
      }
   }
   return 0;
}







