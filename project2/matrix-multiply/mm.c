#include "clock.h"
#define NUM_THREADS 10

int rc;
long t,y;
void* status;
double rate;
clock_t start,end;
struct tms *startTMS, *endTMS;
double cpu_time_used, user_time_used, system_time_used;

/* Initialize the cycle counter */
static unsigned cyc_hi = 0;
static unsigned cyc_lo = 0;


/* Set *hi and *lo to the high and low order bits of the cycle counter.
   Implementation requires assembly code to use the rdtsc instruction. */
void access_counter(unsigned *hi, unsigned *lo) {
  asm("rdtsc; movl %%edx,%0; movl %%eax,%1"
      : "=r" (*hi), "=r" (*lo)
      : /* No input */
      : "%edx", "%eax");
  /* Read cycle counter */
  /* and move results to */
  /* the two outputs */
}


/* Estimate the clock rate by measuring the cycles that elapse */  /* while sleeping for sleeptime seconds */
double mhz(int verbose, int sleeptime)
{
  double rate;
  start_counter();
  sleep(sleeptime);
  rate = get_counter() / (1e6*sleeptime);
  if (verbose)
    printf("Processor clock rate  ̃= %.1f MHz\n", rate);
  return rate;
}

/* Record the current value of the cycle counter. */
void start_counter() {
  access_counter(&cyc_hi, &cyc_lo);
}

/* Return the number of cycles since the last call to start_counter. */
double get_counter() {
  unsigned ncyc_hi, ncyc_lo;
  unsigned hi, lo, borrow;
  double result;

  /* Get cycle counter */
  access_counter(&ncyc_hi, &ncyc_lo);

  /* Do double precision subtraction */
  lo = ncyc_lo - cyc_lo;
  borrow = lo > ncyc_lo;
  hi = ncyc_hi - cyc_hi - borrow;
  result = (double) hi * (1 << 30) * 4 + lo;
  if (result < 0) {
    fprintf(stderr, "Error: counter returns neg value: %.0f\n", result);
  }
  return result;
}


void *mult_matrix(void *t) {
  int m1[3][3], m2[3][3], m3[3][3],i,j,k;
  int sum = 0;
  double result = 0.0;
  printf("Thread %ld starting...\n",(long)t);
  for (i = 0; i < 3; i++) {
    for (j = 0; j < 3; j++) {
      m1[i][j]=25;
      m2[i][j]=25;
    }
  }
  for (i=0;i<=2;i++) {
    for (j=0;j<=2;j++) {
      sum=0;
      for(k=0;k<=2;k++) {
        sum = sum + m1[i][k]*m2[k][j];
      }
      m3[i][j] = sum;
    }
  }
  printf("Thread %ld done. Result = %e\n",(long)t, result);
  pthread_exit((void*) t);
}

int main() {
  startTMS = malloc(sizeof(struct tms));
  endTMS = malloc(sizeof(struct tms));
  times(startTMS);
  start = clock();
  pthread_t thread[NUM_THREADS];
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  for(y=0;y<NUM_THREADS;y++) {
    for(t=0; t<NUM_THREADS; t++) {
      printf("Main: creating thread %ld\n", t);
      rc = pthread_create(&thread[t], &attr, mult_matrix, (void *)t);
      if (rc) {
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        exit(-1);
      }
    }

    for(t=0; t<NUM_THREADS; t++) {
      rc = pthread_join(thread[t], &status);
      if (rc) {
        printf("ERROR; return code from pthread_join() is %d\n", rc);
        exit(-1);
      }
      printf("Main: completed join with thread %ld having a status  of %ld\n",t,(long)status);
    }
  }
  pthread_attr_destroy(&attr);
  end = clock();
  times(endTMS);
  printf("Main: program completed. Exiting.\n");

  rate = mhz(1, 10);
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
  user_time_used = ((double) (endTMS->tms_utime - startTMS->tms_utime)) / CLOCKS_PER_SEC;
  system_time_used = ((double) (endTMS->tms_stime - startTMS->tms_stime)) / CLOCKS_PER_SEC;
  printf("total cpu time used is: %f seconds\n user time is: %f seconds\n system time is: %f seconds\n", cpu_time_used, user_time_used, system_time_used);
  pthread_exit(NULL);

}
