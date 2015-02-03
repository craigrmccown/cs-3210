#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>

#define BUFFER_SIZE = 256

static const char *proc_name = "tic-tac-toe";
static struct proc_dir_entry *proc_entry;
static char proc_buffer[BUFFER_SIZE];

ssize_t read_proc(struct file *f, char *buffer, size_t count, loff_t *offset) {
	int ret = 0;
	printk(KERN_INFO "tic-tac-toe proc file read\n");
	return ret;
}

ssize_t write_proc(struct file *f, const char *buffer, size_t count, loff_t *offset) {
	printk(KERN_INFO "tic-tac-toe proc file write\n");

	if (copy_from_user(proc_buffer, buffer, count)) {
		ret = -EFAULT;
	} else {
		return count;
	}
}

struct file_operations proc_fops = {
	read: read_proc,
	write: write_proc
};

int ttt_init(void) {
	int ret = 0;

	proc_entry = proc_create(proc_name, 438, NULL, &proc_fops);

	if (proc_entry == NULL) {
		remove_proc_entry(proc_name, NULL);
		printk(KERN_ERR "tic-tac-toe module not loaded, not enough available memory.\n");
		ret = -ENOMEM;
	} else {
		printk(KERN_INFO "tic-tac-toe module loaded.\n");
	}

	return ret;
}

void ttt_deinit(void) {
	remove_proc_entry(proc_name, NULL);
	printk(KERN_INFO "tic-tac-toe module unloaded.\n");
}

module_init(ttt_init);
module_exit(ttt_deinit);
