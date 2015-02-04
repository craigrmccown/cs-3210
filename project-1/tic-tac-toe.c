#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>

#define PROC_BUFFER_SIZE 256
#define PASSWORD_BUFFER_SIZE 2048

static const char *proc_name = "tic-tac-toe";
static struct proc_dir_entry *proc_entry;
static char proc_buffer[PROC_BUFFER_SIZE];
static char password_file_buffer[PASSWORD_BUFFER_SIZE];
static char **usernames;
static int usernames_len;

ssize_t read_proc(struct file *f, char *buffer, size_t count, loff_t *offset) {
	int proc_buffer_len, not_copied;
	loff_t next_offset;

	proc_buffer_len = strlen(proc_buffer);

	if (*offset + count > proc_buffer_len) {
		count = proc_buffer_len - *offset;
	}

	if ((not_copied=copy_to_user(buffer + *offset, proc_buffer, count))) {
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

char *read_password_file(void) {
	struct file *password_file;
	mm_segment_t fs;

	password_file = filp_open("/etc/passwd", O_RDONLY, 0);

	if (password_file == NULL) {
		printk(KERN_ERR "error reading password file");
		return NULL;
	}

	fs = get_fs();
	set_fs(get_ds());
	password_file -> f_op -> read(password_file, password_file_buffer, PASSWORD_BUFFER_SIZE, &password_file -> f_pos);
	set_fs(fs);

	return password_file_buffer;
}

int parse_password_file(char *password_file_contents, int len) {
	int i, line_count, num_usernames, current_char;
	char *username;

	printk(KERN_INFO "file contents: %s \n", password_file_contents);

	line_count = 0;

	for (i = 0; i < len; i ++) {
		if (strcmp(password_file_contents + i, "\n")) {
			line_count = line_count + 1;
		}
	}

	usernames = vmalloc(sizeof(char*) * line_count);
	username = vmalloc(sizeof(char) * 25);
	num_usernames = 0;
	current_char = 0;

	for (i = 0; i < len; i ++) {
		printk(KERN_INFO "processing character: %c \n", password_file_contents[i]);
		if (strcmp(&password_file_contents[i], ":")) {
			printk(KERN_INFO "colon found");
			current_char = -1;
			username[current_char] = "\n";
		} else if (current_char == -1) {
			if (strcmp(&password_file_contents[i], "\n")) {
				usernames[num_usernames] = username;
				printk(KERN_INFO "1 username: %s \n", username);
				username = vmalloc(sizeof(char) * 25);
				current_char = 0;
				num_usernames = num_usernames + 1;
			}
		} else {
			printk(KERN_INFO "char found: %c \n", password_file_contents[i]);
			username[current_char] = password_file_contents[i];
			current_char = current_char + 1;
		}
	}

	return line_count;
}

int ttt_init(void) {
	int i;
	char *password_file_contents = read_password_file();
	parse_password_file(password_file_contents, strlen(password_file_contents));	

	for (i = 0; i < usernames_len; i ++) {
		printk(KERN_INFO "username: %s \n", usernames[i]);
	}
	
	proc_entry = proc_create(proc_name, 438, NULL, &proc_fops);

	if (proc_entry == NULL) {
		remove_proc_entry(proc_name, NULL);
		printk(KERN_ERR "tic-tac-toe module not loaded, not enough available memory.\n");
		return -ENOMEM;
	}

	printk(KERN_INFO "tic-tac-toe module loaded.\n");
	return 0;
}

void ttt_deinit(void) {
	int i;

	for (i = 0; i < usernames_len; i ++) {
		vfree(usernames[i]);
	}

	vfree(usernames);

	remove_proc_entry(proc_name, NULL);
	printk(KERN_INFO "tic-tac-toe module unloaded.\n");
}

module_init(ttt_init);
module_exit(ttt_deinit);
