/* taken from WCET lab */
#define EVEN(x) ((x&0x1)==0)

static int gcd_binary(int a, int b)
{    
     unsigned int u = (a < 0) ? (-a) : a;
     unsigned int v = (b < 0) ? (-b) : b;
     int shift= 0;
 
     if (u == 0 || v == 0) { return (u|v); }

     while(EVEN(u)) {
        u >>= 1;
        if(EVEN(v)) {
          v >>= 1;
          shift++;
        }
     }
     while(v > 0) {
       while(EVEN(v)) v >>= 1;
       if(u < v) v -= u;
       else {
            int diff = u-v;
            u = v;
            v = diff;
       }
     }
     return u << shift;
}

int main(int argc, char**argv)
{
  int i,j=0;
  int a=1;
  int b=0;
  for(j = 1; j <= 64; j*=2)
    {
      while(a+b>=a)
        {
          int tmp,r;
          
          tmp = a;      
          a = a+b;
          b = tmp;
		  /* clear cycle counter */
		  asm("sim-clearcycles");
          r = gcd_binary(a*j,j*b);
		  /* print out cycle counter */
		  asm("sim-printcycles");
          if(r != j) return -1;
        }
    }
  return 0;
}
