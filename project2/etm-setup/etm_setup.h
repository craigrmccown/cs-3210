#ifndef ETM_SETUP_H
#define ETM_SETUP_H

#include <linux/slab.h>

#define NUM_THREADS 10
#define NUM_EPOCHS 10

// struct definitions
typedef struct etm_measurement {
	long epoch_id;
	long measurement;
} etm_measurement;

typedef struct etm_data {
	long p_id;
	int num_u_pthread_create_measurements;
	etm_measurement* u_pthread_create_measurements;
} etm_data;

// variable declarations
struct etm_data *execution_time_mod_data;

// function prototypes
int allocate_data_structure(void);

#endif
