#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/times.h>
#include <time.h>

void access_counter(unsigned *hi, unsigned *lo);
void start_counter();
double get_counter();
double mhz(int verbose, int sleeptime);
clock_t times(struct tms *buf);
clock_t clock(void);


