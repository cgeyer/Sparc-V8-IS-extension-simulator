/* taken from WCET lab */
#define EVEN(x) ((x&0x1)==0)

static int gcd_binary(int a, int b)
{    
     unsigned int u = (a < 0) ? (-a) : a;
     unsigned int v = (b < 0) ? (-b) : b;
     int shift= 0;
 
     if(u == 0 || v == 0) { u=v=(u|v); }
    
     int i;
     for(i = 0; i < 31; i++) {
       int even_u = EVEN(u);
       int even_v = EVEN(v);
       if(even_u) u >>= 1;
       if(even_v) v >>= 1;
       if(even_u & even_v) shift++;
     }
     for(i = 0; i < 60; ++i ) {
       int diff = u-v;
       if( (v & 0x1) & (diff < 0)) v = -diff;
       if( (v & 0x1) & (diff >= 0)) u = v;
       if( (v & 0x1) & (diff >= 0)) v = diff;
       v >>= 1;
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
		  /* print out simulated cycles */
		  asm("sim-printcycles");
          if(r != j) return -1;
        }
    }
  return 0;
}
