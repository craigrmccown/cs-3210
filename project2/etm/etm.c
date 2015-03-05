#include "etm.h"

void create_procs(void) {
	root_proc_dir = proc_mkdir("etm", NULL);
	start_proc = proc_create("start", 438, root_proc_dir, &start_fops);
	measurement_proc = proc_create("measurement", 438, root_proc_dir, &measurement_fops);
}

void remove_procs(void) {
	remove_proc_entry("start", root_proc_dir);
	remove_proc_entry("measurement", root_proc_dir);
	remove_proc_entry("etm", NULL);
}

ssize_t write_start_proc(struct file *f, const char *buffer, size_t count, loff_t *offset) {
	char *copied_input;
	long *p_id;

	printk("etm start proc written\n");

	copied_input = kmalloc(sizeof(char) * count + 1, GFP_KERNEL);
	p_id = kmalloc(sizeof(long), GFP_KERNEL);

	if (copied_input == NULL || p_id == NULL) {
		printk("not enough memory to write to etm start proc\n");
		return -ENOMEM;
	}

	if (copy_from_user(copied_input, buffer, count)) {
		printk("failed to copy data from user space when writing to etm start proc\n");
		return -EFAULT;
	}

	etm_free();
	if (!etm_allocate()) {
		printk("not enough memory to reinitialize data structure on write to etm start proc\n");
		return -ENOMEM;
	}

	copied_input[count] = '\0';
	kstrtol(copied_input, 10, p_id);
	execution_time_mod_data -> p_id = *p_id;

	kfree(copied_input);
	kfree(p_id);

	return count;
}

ssize_t write_measurement_proc(struct file *f, const char *buffer, size_t count, loff_t *offset) {
	char *copied_input, *parse_buffer;
	long *p_id, *measurement_id, *epoch_id, *measurement;
	int i, spaces_found, parse_buffer_position;

	printk("etm measurement proc written\n");

	copied_input = kmalloc(sizeof(char) * count, GFP_KERNEL);
	parse_buffer = kmalloc(sizeof(char) * 25, GFP_KERNEL);
	p_id = kmalloc(sizeof(long), GFP_KERNEL);
	measurement_id = kmalloc(sizeof(long), GFP_KERNEL);
	epoch_id = kmalloc(sizeof(long), GFP_KERNEL);
	measurement = kmalloc(sizeof(long), GFP_KERNEL);

	if (
		copied_input == NULL ||
		parse_buffer == NULL ||
		p_id == NULL ||
		measurement_id == NULL ||
		epoch_id == NULL ||
		measurement == NULL
	) {
		printk("not enough memory to write to etm measurement proc\n");
		kfree(copied_input);
		kfree(parse_buffer);
		kfree(p_id);
		kfree(measurement_id);
		kfree(epoch_id);
		kfree(measurement);
		return -ENOMEM;
	}

	if (copy_from_user(copied_input, buffer, count)) {
		printk("failed to copy data from user space when writing to etm measurement proc\n");
		return -EFAULT;
	}

	spaces_found = 0;
	parse_buffer_position = 0;

	for (i = 0; i < count; i ++) {
		if (copied_input[i] == ' ') {
			spaces_found ++;
			parse_buffer[parse_buffer_position] = '\0';
			parse_buffer_position = 0;

			if (spaces_found == 1) {
				kstrtol(parse_buffer, 10, p_id);
			} else if (spaces_found == 2) {
				kstrtol(parse_buffer, 10, measurement_id);
			} else if (spaces_found == 3) {
				kstrtol(parse_buffer, 10, epoch_id);
			} else if (spaces_found == 4) {
				kstrtol(parse_buffer, 10, measurement);
			}
		} else {
			parse_buffer[parse_buffer_position] = copied_input[i];
			parse_buffer_position ++;
		}
	}

	printk("measurment pid: %lu\n", execution_time_mod_data -> p_id);
	printk("passed in pid: %lu\n", *p_id);
	if (*p_id == execution_time_mod_data -> p_id) {
		printk("passed in measurement id: %lu\n", *measurement_id);
		if (*measurement_id == 1) {
			(execution_time_mod_data -> u_pthread_create_measurements)[execution_time_mod_data -> num_u_pthread_create_measurements].epoch_id = *epoch_id;
			(execution_time_mod_data -> u_pthread_create_measurements)[execution_time_mod_data -> num_u_pthread_create_measurements].measurement = *measurement;
			execution_time_mod_data -> num_u_pthread_create_measurements ++;
		}
	}

	kfree(copied_input);
	kfree(parse_buffer);
	kfree(measurement_id);
	kfree(epoch_id);
	kfree(measurement);

	return count;
}

ssize_t read_measurement_proc(struct file *f, char *buffer, size_t count, loff_t *offset) {
	char* results;
	int results_position, measurement_index;

	printk("etm measurement proc read\n");

	if (*offset != 0) {
		return 0;
	}

	results = kmalloc(sizeof(char) * READ_BUFFER_LEN, GFP_KERNEL);
	results_position = 0;

	printk("num pthread create measurements: %i\n", execution_time_mod_data -> num_u_pthread_create_measurements);
	for (measurement_index = 0; measurement_index < (execution_time_mod_data -> num_u_pthread_create_measurements); measurement_index ++) {
		sprintf(
			results + results_position, "epoch: %lu, pthread_create measurement: %lu, clone measurement: %lu \n",
			(execution_time_mod_data -> u_pthread_create_measurements)[measurement_index].epoch_id,
			(execution_time_mod_data -> u_pthread_create_measurements)[measurement_index].measurement,
			(execution_time_mod_data -> k_clone_measurements)[measurement_index]
		);
		results_position = strlen(results);
	}

	if (copy_to_user(buffer, results, READ_BUFFER_LEN)) {
		printk("failed to copy data to user space");
		kfree(results);
		return -EFAULT;
	} else {
		printk("asdf : %s\n", results);
		kfree(results);
		*offset = results_position;
		return results_position;
	}
}

int etm_init(void) {
	create_procs();

	printk("loaded module etm\n");

	return 0;
}

void etm_exit(void) {
	remove_procs();
	printk("unloaded module execution_time_mod\n");
}

struct file_operations start_fops = {
	write: write_start_proc
};

struct file_operations measurement_fops = {
	read: read_measurement_proc,
	write: write_measurement_proc
};

module_init(etm_init);
module_exit(etm_exit);

MODULE_LICENSE("GPL");
