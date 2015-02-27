#include "mm.h"
#define NUM_THREADS 10
#define ROWS 3
#define COLS 3

int rc;
long t,y;
void* status;
clock_t start,end;
double gettime_pthread, gettime_malloc;
FILE *fp_thread, *fp_epoch;
pthread_mutex_t mutexsum;

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
  char thread_data[50];
  char measurement_time[20];
  char real_tid[20];
  char epoch_id[2];
  int x = 0, z = 0, l = 0;
  pd_t *pdata;
  pdata = (pd_t *) threadarg;
  //printf("The ID of this thread is: %u\n", (unsigned int)pthread_self());
  int *array;
  int m1[ROWS][COLS],m2[ROWS][COLS], m3[ROWS][COLS];
  int i,j,k,sum = 0;
  start_timer_malloc();
  array = malloc(sizeof(int)*100000000); // malloc call for system inspection
  gettime_malloc = get_timer_malloc();
  printf("get time for malloc (thread %ld): %f seconds\n",pdata->tid,gettime_malloc);

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
  sprintf(measurement_time, "%f", gettime_malloc);
  sprintf(real_tid, "%u", (unsigned int)pthread_self());
  sprintf(epoch_id, "%ld", pdata->epoch_id);
  pthread_mutex_lock (&mutexsum);
  //fp_thread = fopen("/proc/thread_data", "w");
  //fp_epoch = fopen("/proc/epoch_data", "w");
  //write user data to proc file
  //if (fp_thread == NULL) {
  // printf("can't write to /proc/thread_data\n");
  //} else {
  thread_data[x++] = *epoch_id;
  thread_data[x++] = *" ";
  while(real_tid[z] != *"\0") {
    thread_data[x++] = real_tid[z++];
  }
  z = 0;
  thread_data[x++] = *" ";
  thread_data[x++] = *"3";
  thread_data[x++] = *" ";
  while(measurement_time[z] != *"\0") {
    thread_data[x++] = measurement_time[z++];
  }
  thread_data[x++] = *" ";
  thread_data[x++] = *"\0";
  l = strlen(thread_data);
  for (i = 0; i < l; ++i)
    printf("%c", thread_data[i]);
  printf("\n");
  // fprintf(fp_thread, "%c", thread_data[x]);
  //printf("Thread %ld done. Result = %e\n",(long)t, result);
  free(array);
  pthread_mutex_unlock (&mutexsum);
  pthread_exit((void*) t);

}

int main() {
  pthread_t thread[NUM_THREADS];
  pthread_attr_t attr;
  pthread_mutex_init(&mutexsum, NULL);
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  for(y=0;y<NUM_THREADS;y++) {
    for(t=0; t<NUM_THREADS; t++) {
      //printf("Main: creating thread %ld\n", t);
      pdata = (pd_t *) malloc(sizeof(pd_t));
      pdata->tid = t;
      pdata->epoch_id = y;
      start_timer_pthread();
      rc = pthread_create(&thread[t], &attr, mult_matrix, (void *)pdata);
      gettime_pthread = get_timer_pthread();
      //printf("get time for pthread call (thread %ld): %f seconds\n", t,gettime_pthread);
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
    printf("Should happen after all joins\n");
  }
  pthread_attr_destroy(&attr);
  pthread_mutex_destroy(&mutexsum);
  pthread_exit(NULL);
  printf("Main: program completed. Exiting.\n");
}
