/* taken from WCET lab */
#include "interpolate.h"

static int16_t Tab12_z[12] =
{
	-16640, -16640, -11008, -8192, -512, -512, 1024, 1024, 8192, 11008,
	16640, 16640
} ;


static int16_t Tab12_x[12] =
{
	-4800, -368, -192, -128, -8, -1, 1, 8, 128, 192,
	368, 4800
} ;


static tab Tab12 =
  {
    .Nx = 12,
    .x_table = &Tab12_x[0],
    .z_table = &Tab12_z[0]
  };

static int16_t tab_lookup_sp(const tab * map, int16_t x)
{
   uint8_t N = map->Nx;
   uint16_t Aux_U16;
   uint16_t Aux_U16_a;
   uint16_t t;


   const int16_t * x_table_safe, * x_table;
   const int16_t * z_table ;

   x_table_safe = (const int16_t *) map->x_table;
   z_table = (const int16_t *) map->z_table;

   int i, p = 0;
   x_table = x_table_safe+1;
   for(i = 1; i < N-1; i++)
   {
     if(x > *(x_table++)) p++;
   }
   int16_t x0,x1,v0,v1;

   x_table = x_table_safe + p;
   x0 = *(x_table++);
   x1 = *x_table;
   if(x > map->x_table[N-1])x = x1;
   if(x < map->x_table[0])  x = x0;
   
   z_table += p;
   v0 = *(z_table++);
   v1 = *z_table;

   Aux_U16 = (uint16_t) (((uint16_t) x) - ((uint16_t) x0));
   Aux_U16_a = (uint16_t) (((uint16_t) x1) - ((uint16_t) x0));

   t = ((uint16_t) (uint32_t) 
       (((uint32_t) (
           ((uint16_t) (((uint16_t) v1) - ((uint16_t) v0))) 
             * 
           ((uint32_t) Aux_U16))) 
         / 
        ((uint32_t) Aux_U16_a)));

   if (v0 > v1) t=-t;
   v0 += t;

   return v0;
}

int main() {
  int i;
  int test_out_counter = 0;
  int16_t result;
  for(i=-400;i<400;i+=30) {
    asm("sim-clearcycles");
    result = tab_lookup_sp(&Tab12,i);
    asm("sim-printcycles");
    if (result != test_data[test_out_counter]) {
	  return -1;
	}
	test_out_counter++;
  }
  return 0;
}
