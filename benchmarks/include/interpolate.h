#include <stdint.h>

typedef struct tab_struct {
   uint8_t Nx;
   const int16_t * x_table;
   const int16_t * z_table;
} tab;

/* chose array length such that next data structure 
   is aligned in memory */
#define TEST_DATA_LENGTH 28

int16_t test_data[TEST_DATA_LENGTH] = {
	-16640, -16640, -15744, -14784, -13824, 
	-12864, -11904, -10920, -9600, -8280, 
	-6400, -4480, -2560,  -640,   1740, 
	3532, 5324, 7116, 8720, 10040, 
	11264, 12224, 13184, 14144, 15104, 
	16064, 16640, 0
};
