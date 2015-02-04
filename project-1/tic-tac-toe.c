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

	proc_buffer_len = strlen(proc_buffer + *offset);

	if (*offset + count > proc_buffer_len) {
		count = proc_buffer_len - *offset;
	}

	not_copied = copy_to_user(buffer + *offset, proc_buffer + *offset, count);

	next_offset = *offset + (count - not_copied);
	*offset = next_offset;

	return count - not_copied;
}

ssize_t write_proc(struct file *f, const char *buffer, size_t count, loff_t *offset) {
	int remaining_bytes, not_copied;
	loff_t next_offset;

	remaining_bytes = strlen(buffer + *offset);

	if (*offset + remaining_bytes > BUFFER_SIZE) {
		remaining_bytes = BUFFER_SIZE - *offset;
	}

	not_copied = copy_from_user(proc_buffer + *offset, buffer + *offset, remaining_bytes);
	next_offset = *offset + (remaining_bytes - not_copied);

	return remaining_bytes - not_copied;
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
