#ifndef ETM_SETUP
#define ETM_SETUP

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

// function definitions
int allocate_data_structure(void) {
	execution_time_mod_data = kmalloc(sizeof(etm_data), GFP_KERNEL);

	if (execution_time_mod_data == NULL) {
		return 0;
	}

	execution_time_mod_data -> p_id = -1;
	execution_time_mod_data -> num_u_pthread_create_measurements = 0;
	execution_time_mod_data -> u_pthread_create_measurements = kmalloc(sizeof(etm_measurement) * NUM_EPOCHS * NUM_THREADS, GFP_KERNEL);

	if (execution_time_mod_data == NULL) {
		kfree(execution_time_mod_data);
		return 0;
	}

	return 1;
}

void etm_setup(void) {
	if (allocate_data_structure()) {
		EXPORT_SYMBOL_GPL(etm_measurement);
		EXPORT_SYMBOL_GPL(etm_data);
		EXPORT_SYMBOL_GPL(execution_time_mod_data);
	}
}

etm_setup();

#endif
