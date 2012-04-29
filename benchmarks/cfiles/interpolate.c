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

static int16_t tab_lookup(const tab * map, int16_t x)
{
   int16_t Aux_S16;
   int16_t Aux_S16_a;
   uint16_t Aux_U16;
   uint16_t Aux_U16_a;
   uint16_t t;


   const int16_t * x_table ;
   const int16_t * z_table ;

   x_table = (const int16_t *) map->x_table;
   z_table = (const int16_t *) map->z_table;

   if (x <= *(x_table)) {

      return z_table[0];
   }
   if (x >= x_table[(uint8_t)(map->Nx - 1)]) {
      return z_table[(uint8_t) (map->Nx - 1)];
   }

   (x_table)++;
   while (x > *((x_table)++)) {
      (z_table)++;
   }
   x_table -= 2 ;
   Aux_S16 = *((z_table)++);
   Aux_S16_a = *(z_table);


   Aux_U16 = (uint16_t) (((uint16_t) x) - ((uint16_t) x_table[0]));
   Aux_U16_a = (uint16_t) (((uint16_t) x_table[1]) - ((uint16_t) x_table[0]));
   t = ((uint16_t) (uint32_t) 
       (((uint32_t) (
           ((uint16_t) (((uint16_t) Aux_S16_a) - ((uint16_t) Aux_S16))) 
             * 
           ((uint32_t) Aux_U16))) 
         / 
        ((uint32_t) Aux_U16_a)));
   if (Aux_S16 <= Aux_S16_a) {
      Aux_S16 += t;
   }
   else {
      Aux_S16 -= t;
   }
   return Aux_S16;
}

int main() {
  int i;
  int test_out_counter = 0;
  int16_t result;
  for(i=-400;i<400;i+=30) {
    asm("sim-clearcycles");
    result = tab_lookup(&Tab12,i);
    asm("sim-printcycles");
    if (result != test_data[test_out_counter]) {
	  return -1;
	}
	test_out_counter++;
  }
  return 0;
}
