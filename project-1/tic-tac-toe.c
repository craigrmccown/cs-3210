#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>

#define BUFFER_SIZE 256

static const char *proc_name = "tic-tac-toe";
static struct proc_dir_entry *proc_entry;
static char proc_buffer[BUFFER_SIZE];

ssize_t read_proc(struct file *f, char *buffer, size_t count, loff_t *offset) {
int proc_buffer_len, not_copied;
loff_t next_offset;

	proc_buffer_len = strlen(proc_buffer);

	if (*offset + count > proc_buffer_len) {
		count = proc_buffer_len - *offset;
	}

	if (not_copied=copy_to_user(buffer + *offset, proc_buffer, count)) {
		count = -EFAULT;
	} 
	next_offset = *offset + (count - not_copied);
	*offset = next_offset;

	return count-not_copied;
}

ssize_t write_proc(struct file *f, const char *buffer, size_t count, loff_t *offset) {
printk(KERN_INFO "tic-tac-toe proc file write: %s \n", buffer);

	if (copy_from_user(proc_buffer, buffer, strlen(buffer))) {
		return -EFAULT;
	} else {
		proc_buffer[count] = 0;
		printk(KERN_INFO "written to buffer: %s \n", proc_buffer);
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
