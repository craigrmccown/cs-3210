#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/times.h>
#include <time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

void access_counter(unsigned *hi, unsigned *lo);
void start_counter();
double get_counter();
double mhz(int verbose, int sleeptime);
clock_t times(struct tms *buf);
clock_t clock(void);
int gettimeofday(struct timeval *tv, void* tz);
typedef struct pthread_data {
  long tid;
  long epoch_id;
} pd_t;





