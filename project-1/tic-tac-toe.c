#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>

static const char *proc_name = "tic-tac-toe";
static struct proc_dir_entry *proc_entry;

ssize_t read_tic_tac_toe(struct file *f, char *buffer, size_t count, loff_t *offset) {
	int ret = 0;
	printk(KERN_INFO "tic-tac-toe proc file read");
	return ret;
}

ssize_t write_tic_tac_toe(struct file *f, const char *buffer, size_t count, loff_t *offset) {
	int ret = 0;
	printk(KERN_INFO "tic-tac-toe proc file write");
	return ret;
}

struct file_operations proc_fops = {
	read: read_tic_tac_toe,
	write: write_tic_tac_toe
};

int init_tic_tac_toe(void) {
	int ret = 0;

	proc_entry = proc_create(proc_name, 0, NULL, &proc_fops);

	if (proc_entry == NULL) {
		printk(KERN_ERR "tic-tac-toe module not loaded, not enough available memory.\n");
		ret = -ENOMEM;
	} else {
		printk(KERN_INFO "tic-tac-toe module loaded.\n");
	}

	return ret;
}

void deinit_tic_tac_toe(void) {
	remove_proc_entry(proc_name, NULL);
	printk(KERN_INFO "tic-tac-toe module unloaded.\n");
}

module_init(init_tic_tac_toe);
module_exit(deinit_tic_tac_toe);
