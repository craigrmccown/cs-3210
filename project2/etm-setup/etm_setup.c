#include <linux/etm_setup.h>

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

if (allocate_data_structure()) {
	EXPORT_SYMBOL_GPL(etm_measurement);
	EXPORT_SYMBOL_GPL(etm_data);
	EXPORT_SYMBOL_GPL(execution_time_mod_data);
}
