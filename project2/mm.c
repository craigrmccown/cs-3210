#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#define NUM_THREADS 10

int rc;
long t,y;
void* status;

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
  printf("Main: program completed. Exiting.\n");
  pthread_exit(NULL);
}
