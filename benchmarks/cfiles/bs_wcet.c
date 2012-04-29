/* taken from WCET lab */
#define TESTSIZE 16
static int TESTDATA[TESTSIZE] = { 2, 4, 6, 8, 10, 12, 14, 16, 18, 20,
                           22, 24, 26, 28, 30, 32 };

static int bs(int*keys, int key, int size)
{
  int i;
  int left = 0, right = size-1;
  int idx = (right + left) >> 1;

  for(i = 0; i < sizeof(int)*8; i++) { 
    right = (key < keys[idx] ? idx-1 : right);
    left  = (key > keys[idx] ? idx+1 : left);
    idx   = (right + left) >> 1;
  }
  return (keys[idx] == key) ? idx : -1;
}

int main(int argc, char** argv)
{
  int i,j,ix;
  i = TESTSIZE;
  for(i = 1; i <= TESTSIZE; i++)
    {      
      for(j = 1; j < i*2 + 1; j++) {
        int r;

        ix = (j-1)>>1;

		/* clear cycles for simulator */
		asm("sim-clearcycles");
        r = bs(TESTDATA, j, i);
		/* print out simulated cycles so far */
		asm("sim-printcycles");

        if((j % 2) == 1) {
          if(r != -1) {
            return -1;
          }
        } else {
          if(r != ix) {
            return -1;
          }
        }
      }
    }
  return 0;
}
