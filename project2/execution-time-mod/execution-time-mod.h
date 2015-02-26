#ifndef EXECUTION_TIME_MODULE_H
#define EXECUTION_TIME_MODULE_H

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>

// struct declarations
struct thread_time_data {
	int thread_id;
	double total_wait;
	double u_malloc_time;
	double u_free_time;
	double u_create_time;
};

struct epoch_time_data {
	int epoch_id;
	double total_wait;
	struct thread_time_data threads[10];
};

struct time_data {
	int clock_type;
	struct epoch_time_data epochs[10];
};

// variable declarations
struct file_operations time_data_fops;
struct file_operations epoch_data_fops;
struct file_operations thread_data_fops;
struct proc_dir_entry *root_proc_dir;
struct proc_dir_entry *time_data_proc;
struct proc_dir_entry *epoch_data_proc;
struct proc_dir_entry *thread_data_proc;

// function prototypes
ssize_t read_execution_times(struct file *f, char *buffer, size_t count, loff_t *offset);
ssize_t write_time_data(struct file *f, const char *buffer, size_t count, loff_t *offset);
ssize_t write_epoch_data(struct file *f, const char *buffer, size_t count, loff_t *offset);
ssize_t write_thread_data(struct file *f, const char *buffer, size_t count, loff_t *offset);
void execution_time_exit(void);
int execution_time_init(void);

#endif
