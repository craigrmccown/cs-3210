#include "execution-time-mod.h"

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

ssize_t write_time_data(struct file *f, const char *buffer, size_t count, loff_t *offset) {
	int spaces_found, i;
	char *user_input;

	if (copy_from_user(user_input, buffer, count)) {
		return -EFAULT;
	}

	spaces_found = 0;

	for (i = 0; i < count; i ++) {
	}

	return count;
}

ssize_t write_epoch_data(struct file *f, const char *buffer, size_t count, loff_t *offset) {
	int spaces_found, i;
	char *user_input;

	if (copy_from_user(user_input, buffer, count)) {
		return -EFAULT;
	}

	spaces_found = 0;

	for (i = 0; i < count; i ++) {
	}

	return count;
}

ssize_t write_thread_data(struct file *f, const char *buffer, size_t count, loff_t *offset) {
	int spaces_found, i;
	char *user_input;

	if (copy_from_user(user_input, buffer, count)) {
		return -EFAULT;
	}

	spaces_found = 0;

	for (i = 0; i < count; i ++) {
	}

	return count;
}

int execution_time_init(void) {
	root_proc_dir = proc_mkdir("execution_time", NULL);
	time_data_proc = proc_create("time_data", 438, root_proc_dir, &time_data_fops);
	epoch_data_proc = proc_create("epoch_data", 438, root_proc_dir, &epoch_data_fops);
	thread_data_proc = proc_create("thread_data", 438, root_proc_dir, &thread_data_fops);

	return 0;
}

void execution_time_exit(void) {
	remove_proc_entry("thread_data", root_proc_dir);
	remove_proc_entry("epoch_data", root_proc_dir);
	remove_proc_entry("time_data", root_proc_dir);
	remove_proc_entry("execution_time", NULL);
}

struct file_operations time_data_fops = {
	read: read_execution_times,
	write: write_time_data
};

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
