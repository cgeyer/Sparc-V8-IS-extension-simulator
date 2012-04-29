/*
 * dijkstra_sp.c
 * WCET Analysis Lab
 */

#define NRNODES 7
#define INFINITE 998

static int cost[NRNODES][NRNODES]={
  {INFINITE,2,4,7,INFINITE,5,INFINITE},
  {2,INFINITE,INFINITE,6,3,INFINITE,8},
  {4,INFINITE,INFINITE,INFINITE,INFINITE,6,INFINITE},
  {7,6,INFINITE,INFINITE,INFINITE,1,6},
  {INFINITE,3,INFINITE,INFINITE,INFINITE,INFINITE,7},
  {5,INFINITE,6,1,INFINITE,INFINITE,6},
  {INFINITE,8,INFINITE,6,7,6,INFINITE}};
static int cost_answers[NRNODES]={
  0,  2,  4,  6,  5,  5,  10
};
static int cost2[NRNODES][NRNODES]= {
  {INFINITE,2,INFINITE,INFINITE,INFINITE,INFINITE,INFINITE},
  {2,INFINITE,2,INFINITE,INFINITE,INFINITE,INFINITE},
  {INFINITE,2,INFINITE,2,INFINITE,INFINITE,INFINITE},
  {INFINITE,INFINITE,2,INFINITE,2,INFINITE,INFINITE},
  {INFINITE,INFINITE,INFINITE,2,INFINITE,2,INFINITE},
  {INFINITE,INFINITE,INFINITE,INFINITE,2,INFINITE,2},
  {INFINITE,INFINITE,INFINITE,INFINITE,INFINITE,2,INFINITE}};
static int cost2_answers[NRNODES]={
  0,  2,  4,  6,  8,  10,  12
};
static int cost3[NRNODES][NRNODES]= {
  {INFINITE,6,5,4,3,2,1},
  {6,INFINITE,5,4,3,2,1},
  {5,5,INFINITE,4,3,2,1},
  {4,4,4,INFINITE,3,2,1},
  {3,3,3,3,INFINITE,2,1},
  {2,2,2,2,2,INFINITE,1},
  {1,1,1,1,1,1,INFINITE}};
static int cost3_answers[NRNODES]={  
  0,  2,  2,  2,  2,  2,  1 
};
static int cost4[NRNODES][NRNODES]= {
  {INFINITE,1,50,50,50,50,50},
  {1,INFINITE,1,48,48,48,48},
  {50,1,INFINITE,1,46,46,46},
  {50,48,1,INFINITE,1,44,44},
  {50,48,46,1,INFINITE,1,42},
  {50,48,46,44,1,INFINITE,1},
  {50,48,46,44,42,1,INFINITE}};
static int cost4_answers[NRNODES]={
  0,  1,  2,  3,  4,  5,  6 
};
static int cost5[NRNODES][NRNODES]= {
  {INFINITE,6,5,4,3,2,1},
  {6,INFINITE,INFINITE,INFINITE,INFINITE,INFINITE,INFINITE},
  {5,INFINITE,INFINITE,INFINITE,INFINITE,INFINITE,INFINITE},
  {4,INFINITE,INFINITE,INFINITE,INFINITE,INFINITE,INFINITE},
  {3,INFINITE,INFINITE,INFINITE,INFINITE,INFINITE,INFINITE},
  {2,INFINITE,INFINITE,INFINITE,INFINITE,INFINITE,INFINITE},
  {1,INFINITE,INFINITE,INFINITE,INFINITE,INFINITE,INFINITE}};
static int cost5_answers[NRNODES]={
  0,  6,  5,  4,  3,  2,  1
};

static void dijkstra(int cost[][NRNODES],int *preced, int *distance)
{
    int selected[NRNODES]={0}, dc;
    volatile int current=0,k=0,i,j,smalldist,newdist,temp, preced_temp, dist_temp;

    /* ai: loop (here) loops exactly (NRNODES) ; */
    for(i=0;i<NRNODES;i++)
       distance[i]=INFINITE;

    selected[current]=1;
    distance[0]=0;
    current=0;

    /* ai: loop (here) loops exactly (NRNODES-1) ; */
    for(j=0;j<NRNODES-1;j++)
    {
        smalldist=INFINITE;
        dc=distance[current];
        /* ai: loop (here) loops exactly NRNODES ; */
        for(i=0;i<NRNODES;i++)
        {
             	  // bei jedem durchgang der while -1
               	  // ai: flow (here) <= NRNODES * (NRNODES-1) / 2 ("dijkstra");
                  // haengt nur von anzahl der nodes ab und nicht von beschaffenheit der distanzen
            	  newdist= dc+cost[current][i];

            	  preced_temp = preced[i];
            	  dist_temp = distance[i];
            	  temp = (newdist < dist_temp) ? current : preced_temp;
                  preced[i]=(selected[i]==0) ? temp : preced_temp;
                  temp = (newdist < dist_temp) ? newdist : dist_temp;
                  distance[i]=(selected[i]==0) ? temp : dist_temp;

                  dist_temp = distance[i];
                  temp = (dist_temp < smalldist) ? i : k;
                  k=(selected[i]==0) ? temp : k;
                  temp = (dist_temp < smalldist) ? dist_temp : smalldist;
                  smalldist=(selected[i]==0) ? temp : smalldist;
        }
        current=k;
        selected[current]=1;
   }
}


int main(int argc, char** argv)
{    

  int i,preced[NRNODES]={0},distance[NRNODES];


  /* clear simulator cycle counter */
  asm("sim-clearcycles");
  dijkstra(cost,preced,distance);
  /* print out cycle counter */
  asm("sim-printcycles");
    
  for(i=0;i<NRNODES;i++) {
    if (distance[i] != cost_answers[i]) {
      return -1;
    }
  }
  
  /* clear simulator cycle counter */
  asm("sim-clearcycles");
  dijkstra(cost2,preced,distance);
  /* print out cycle counter */
  asm("sim-printcycles");
  
  for(i=0;i<NRNODES;i++) {
    if (distance[i] != cost2_answers[i]) {
      return -1;
    }
  }
  
  /* clear simulator cycle counter */
  asm("sim-clearcycles");
  dijkstra(cost3,preced,distance);
  /* print out cycle counter */
  asm("sim-printcycles");
  
  for(i=0;i<NRNODES;i++) {
    if (distance[i] != cost3_answers[i]) {
      return -1;
    }
  }
  
  /* clear simulator cycle counter */
  asm("sim-clearcycles");
  dijkstra(cost4,preced,distance);
  /* print out cycle counter */
  asm("sim-printcycles");
  
  for(i=0;i<NRNODES;i++) {
    if (distance[i] != cost4_answers[i]) {
      return -1;
    }
  }
  
  /* clear simulator cycle counter */
  asm("sim-clearcycles");
  dijkstra(cost5,preced,distance);
  /* print out cycle counter */
  asm("sim-printcycles");
  
  for(i=0;i<NRNODES;i++) {
    if (distance[i] != cost5_answers[i]) {
      return -1;
    }
  }

  return 0;

}
