#include "execution-time-mod.h"

struct epoch_time_data *find_epoch_time_data(long epoch_id) {
	int i;
	struct epoch_time_data *epoch;

	for (i = 0; i < execution_time_data -> num_epochs; i ++) {
		if ((execution_time_data -> epochs)[i] -> epoch_id == epoch_id) {
			return (execution_time_data -> epochs)[i];
		}
	}

	epoch = kmalloc(sizeof(struct epoch_time_data), GFP_KERNEL);

	if (epoch != NULL) {
		epoch -> epoch_id = epoch_id;
		(execution_time_data -> epochs)[i] = epoch;
		(execution_time_data -> num_epochs) ++;
	}

	return epoch;
}

ssize_t read_execution_times(struct file *f, char *buffer, size_t count, loff_t *offset) {
	char* results;
	int results_len;

	if (offset == 0) {
		if (copy_to_user(buffer, results, results_len)) {
			return -EFAULT;
		} else {
			return results_len;
		}
	} else {
		return 0;
	}
}

ssize_t write_epoch_data(struct file *f, const char *buffer, size_t count, loff_t *offset) {
	return count;
}

ssize_t write_thread_data(struct file *f, const char *buffer, size_t count, loff_t *offset) {
	int spaces_found, i, input_buffer_position;
	long *epoch_id, thread_id, measurement_id, measurement;
	char *user_input, input_buffer;
	struct epoch_time_data *epoch;
	struct thread_time_data *thread;

	input_buffer = kmalloc(sizeof(char) * 50, GFP_KERNEL);

	if (input_buffer == NULL) {
		return -EFAULT;
	}

	if (copy_from_user(user_input, buffer, count)) {
		return -EFAULT;
	}

	spaces_found = 0;
	input_buffer_position = 0;

	for (i = 0; i < count; i ++) {
		if (user_input[i] == ' ') {
			spaces_found ++;
			input_buffer[input_buffer_position] = '\0';
			input_buffer_position = 0;

			if (spaces_found == 1) {
				kstrtol(input_buffer, 10, epoch_id);
			} else if (spaces_found == 2) {
				kstrtol(input_buffer, 10, thread_id);
			} else if (spaces_found == 3) {
				kstrtol(input_buffer, 10, measurement_id);
			} else if (spaces_found == 4) {
				kstrtol(input_buffer, 10, measurement);
			}
		} else {
			input_buffer[input_buffer_position] = user_input[i];
			input_buffer_position ++;
		}
	}

	epoch = find_epoch_time_data(*epoch_id);

	if (epoch == NULL) {
		return -EFAULT;
	}

	thread = find_thread_time_data(*epoch_id);

	if (thread == NULL) {
		return -EFAULT;
	}

	if (*measurement_id == 0) {
		thread -> total_wait = *measurement;
	} else if (*measurement_id == 1) {
		thread -> u_malloc_time = *measurement;
	} else if (*measurement_id == 2) {
		thread -> u_free_time = *measurement;
	} else if (*measurement_id == 3) {
		thread -> u_create_time = *measurement;
	}

	return count;
}

int execution_time_init(void) {
	execution_time_data = kmalloc(sizeof(struct time_data), GFP_KERNEL);

	if (execution_time_data == NULL) {
		return -EFAULT;
	}

	execution_time_data -> num_epochs = 0;

	root_proc_dir = proc_mkdir("execution_time", NULL);
	epoch_data_proc = proc_create("epoch_data", 438, root_proc_dir, &epoch_data_fops);
	thread_data_proc = proc_create("thread_data", 438, root_proc_dir, &thread_data_fops);

	return 0;
}

void execution_time_exit(void) {
	kfree(execution_time_data);

	remove_proc_entry("thread_data", root_proc_dir);
	remove_proc_entry("epoch_data", root_proc_dir);
	remove_proc_entry("execution_time", NULL);
}

struct file_operations epoch_data_fops = {
	read: read_execution_times,
	write: write_epoch_data 
};

struct file_operations thread_data_fops = {
	read: read_execution_times,
	write: write_thread_data
};

module_init(execution_time_init);
module_exit(execution_time_exit);
