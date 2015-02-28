#include "mm.h"
#define USED_CLOCK CLOCK_PROCESS_CPUTIME_ID
#define NANOS 1000000000LL
#define NUM_THREADS 10
#define ROWS 3
#define COLS 3

int rc;
long t,y,pid,start,elapsed,microseconds;
void* status;
double gettime_pthread, gettime_malloc;
FILE *fp_thread, *fp_start;
char pid_start[20];
struct timespec begin, current;

/* Initialize the cycle counter */
static unsigned cyc_hi = 0;
static unsigned cyc_lo = 0;
static struct timeval tstart_pthread, tstart_malloc;
pd_t *pdata;




/* Record current time */
void start_timer_pthread()
{
  gettimeofday(&tstart_pthread, NULL);
}

void start_timer_malloc()
{
  gettimeofday(&tstart_malloc, NULL);
}

/* Get number of seconds since last call to start_timer */
double get_timer_pthread()
{
  struct timeval tfinish;
  long sec, usec;

  gettimeofday(&tfinish, NULL);
  sec = tfinish.tv_sec - tstart_pthread.tv_sec;
  usec = tfinish.tv_usec - tstart_pthread.tv_usec;
  return sec + 1e-6*usec;
}

double get_timer_malloc()
{
  struct timeval tfinish;
  long sec, usec;

  gettimeofday(&tfinish, NULL);
  sec = tfinish.tv_sec - tstart_malloc.tv_sec;
  usec = tfinish.tv_usec - tstart_malloc.tv_usec;
  return sec + 1e-6*usec;
}

void *mult_matrix(void *threadarg) {
  pd_t *pdata;
  pdata = (pd_t *) threadarg;
  int m1[ROWS][COLS],m2[ROWS][COLS], m3[ROWS][COLS];
  int i,j,k,sum = 0;

  double result = 0.0;
  //printf("Thread %ld starting...\n",(long)t);
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
  pthread_exit((void*) t);
}

int main() {
  pid = getpid();
  pthread_t thread[NUM_THREADS];
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  sprintf(pid_start, "%ld", pid);
  fp_start = fopen("/proc/etm/start", "w");
  if (fp_start == NULL) {
    printf("can't write to /proc/etm/start");
  } else {
    fprintf(fp_start, "%s", pid_start);
    fclose(fp_start);
  }
  for(y=0;y<NUM_THREADS;y++) {
    for(t=0; t<NUM_THREADS; t++) {
      char thread_data[50];
      char measurement_time[20];
      char real_pid[20];
      char epoch_id[2];
      int i = 0, j = 0;
      printf("Main: creating thread %ld\n", t);
      pdata = (pd_t *) malloc(sizeof(pd_t));
      pdata->pid = pid;
      pdata->epoch_id = y;
      start_timer_pthread();
      clock_gettime(USED_CLOCK,&begin);
      rc = pthread_create(&thread[t], &attr, mult_matrix, (void *)pdata);
      gettime_pthread = get_timer_pthread();
      clock_gettime(USED_CLOCK,&current);
      start = begin.tv_sec*NANOS + begin.tv_nsec;
      elapsed = current.tv_sec*NANOS + current.tv_nsec - start;
      microseconds = elapsed / 1000 + (elapsed % 1000 >= 500);
      if (rc) {
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        exit(-1);
      }
      //printf("get time for pthread call (thread %ld): %f seconds\n", t,gettime_pthread);
      sprintf(measurement_time, "%lu", (long)(gettime_pthread * 1e6));
      sprintf(real_pid, "%ld", pdata->pid);
      sprintf(epoch_id, "%ld", pdata->epoch_id);
      fp_thread = fopen("/proc/etm/measurement", "w");
      if (fp_thread == NULL) {
        printf("can't write to /proc/etm/measurement\n");
      } else {
        while(real_pid[j] != '\0') {
          thread_data[i++] = real_pid[j++];
        }
        thread_data[i++] = ' ';
        thread_data[i++] = '1';
        thread_data[i++] = ' ';
        thread_data[i++] = *epoch_id;
        thread_data[i++] = ' ';
        j = 0;
        while(measurement_time[j] != '\0') {
          thread_data[i++] = measurement_time[j++];
        }
        thread_data[i++] = ' ';
        thread_data[i++] = '\0';
        fprintf(fp_thread, "%s", thread_data);
        //printf("%s\n", thread_data);
        fclose(fp_thread);
      }
    }
    for(t=0; t<NUM_THREADS; t++) {
      rc = pthread_join(thread[t], &status);
      if (rc) {
        printf("ERROR; return code from pthread_join() is %d\n", rc);
        exit(-1);
      }
      // printf("Main: completed join with thread %ld having a status  of %ld\n",t,(long)status);
    }
    printf("Should happen after all joins\n");
  }
  pthread_attr_destroy(&attr);
  free(pdata);
  pthread_exit(NULL);
  printf("Main: program completed. Exiting.\n");
}
