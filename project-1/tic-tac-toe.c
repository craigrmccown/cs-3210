#include "linux/module.h"
#include "linux/proc_fs.h"

static const char *proc_name = "tic-tac-toe";
static struct proc_dir_entry *proc_entry;
static struct *proc_fops;

int read_tic_tac_toe(struct *file f, const char __user *user, size_t buffer_size, loff_t *offset) {
	int ret = 0;
	return ret;
}


int init_tic_tac_toe(void) {
	int ret = 0;

	proc_entry = proc_create(proc_name, 0, NULL, proc_fops);

	if (proc_entry == NULL) {
		printk(KERN_ERR "Tic-Tac-Toe module not loaded, not enough available memory.\n");
		ret = -ENOMEM;
	} else {
		proc_fops = malloc(sizeof(proc_dir_entry));

		if (proc_fops = NULL) {
			printk(KERN_ERR "Tic-Tac-Toe module not loaded, not enough available memory.\n");
			ret = -ENOMEM;
		} else {
			proc_fops -> read = read_tic_tac_toe
			printk(KERN_INFO "Tic-Tac-Toe module loaded.\n");
		}
	}

	return ret;
}

void deinit_tic_tac_toe(void) {
	printk(KERN_INFO "Tic-Tac-Toe module unloaded.\n");
}

module_init(init_tic_tac_toe);
module_exit(deinit_tic_tac_toe);
