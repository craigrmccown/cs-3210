#ifndef EXECUTION_TIME_MODULE_H
#define EXECUTION_TIME_MODULE_H

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/gfp.h>
#include <linux/etm_setup.h>

#define NUM_THREADS 10
#define NUM_EPOCHS 10
#define READ_BUFFER_LEN 10000

// variable declarations
struct file_operations start_fops;
struct file_operations measurement_fops;
struct proc_dir_entry *root_proc_dir;
struct proc_dir_entry *start_proc;
struct proc_dir_entry *measurement_proc;
extern struct etm_data *execution_time_mod_data;

// function prototypes
int allocate_data_structure(void);
void free_data_structure(void);
void create_procs(void);
void remove_procs(void);
ssize_t write_start_proc(struct file *f, const char *buffer, size_t count, loff_t *offset);
ssize_t write_measurement_proc(struct file *f, const char *buffer, size_t count, loff_t *offset);
ssize_t read_measurement_proc(struct file *f, char *buffer, size_t count, loff_t *offset);
int etm_init(void);
void etm_exit(void);

#endif
