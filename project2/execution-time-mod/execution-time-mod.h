#ifndef EXECUTION_TIME_MODULE_H
#define EXECUTION_TIME_MODULE_H

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/gfp.h>

#define NUM_THREADS 10
#define NUM_EPOCHS 10

// struct declarations
struct thread_time_data {
	long thread_id;
	long total_wait;
	long u_malloc_time;
	long u_free_time;
	long u_create_time;
};

struct epoch_time_data {
	long epoch_id;
	long total_wait;
	int num_threads;
	struct thread_time_data threads[NUM_THREADS];
};

struct time_data {
	int clock_type;
	int num_epochs;
	struct epoch_time_data epochs[NUM_EPOCHS];
};

// variable declarations
struct file_operations epoch_data_fops;
struct file_operations thread_data_fops;
struct proc_dir_entry *root_proc_dir;
struct proc_dir_entry *epoch_data_proc;
struct proc_dir_entry *thread_data_proc;
struct time_data *execution_time_data;

// function prototypes
struct epoch_time_data *find_epoch_time_data(long epoch_id);
struct thread_time_data *find_thread_time_data(long thread_id);
ssize_t read_execution_times(struct file *f, char *buffer, size_t count, loff_t *offset);
ssize_t write_epoch_data(struct file *f, const char *buffer, size_t count, loff_t *offset);
ssize_t write_thread_data(struct file *f, const char *buffer, size_t count, loff_t *offset);
void execution_time_exit(void);
int execution_time_init(void);

#endif
