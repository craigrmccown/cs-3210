#include "ttt-module.h"

ssize_t read_game(struct file *f, char *buffer, size_t count, loff_t *offset) {
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

ssize_t write_game(struct file *f, const char *buffer, size_t count, loff_t *offset) {
	if (copy_from_user(proc_buffer, buffer, strlen(buffer))) {
		return -EFAULT;
	} else {
		proc_buffer[count] = 0;
		return count;
	}
}

ssize_t read_opponent(struct file *f, char *buffer, size_t count, loff_t *offset) {
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

ssize_t write_opponent(struct file *f, const char *buffer, size_t count, loff_t *offset) {
	if (copy_from_user(proc_buffer, buffer, strlen(buffer))) {
		return -EFAULT;
	} else {
		proc_buffer[count] = 0;
		return count;
	}
}

char *read_password_file(void) {
	struct file *password_file;
	mm_segment_t fs;

	password_file = filp_open("/etc/passwd", O_RDONLY, 0);

	if (password_file == NULL) {
		return NULL;
	}

	fs = get_fs();
	set_fs(get_ds());
	password_file -> f_op -> read(password_file, password_file_buffer, PASSWORD_BUFFER_SIZE, &password_file -> f_pos);
	set_fs(fs);

	return password_file_buffer;
}

int parse_password_file(char *password_file_contents, int len) {
	int i, line_count, current_char;
	char *username;

	line_count = 0;

	for (i = 0; i < len; i ++) {
		if (password_file_contents[i] == '\n') {
			line_count = line_count + 1;
		}
	}

	usernames = vmalloc(sizeof(char*) * line_count);
	username = vmalloc(sizeof(char) * 25);
	num_users = 0;
	current_char = 0;

	for (i = 0; i < len; i ++) {
		if (current_char == -1) {
			if (password_file_contents[i] == '\n') {
				usernames[num_users] = username;
				username = vmalloc(sizeof(char) * 25);
				current_char = 0;
				num_users = num_users + 1;
			}
		} else if (password_file_contents[i] == ':') {
			username[current_char] = '\0';
			current_char = -1;
		} else {
			username[current_char] = password_file_contents[i];
			current_char = current_char + 1;
		}
	}

	return line_count;
}

int ttt_init(void) {
	int i;
	char *password_file_contents;
	struct proc_dir_entry *user_proc_dir;
	struct proc_dir_entry *game_proc_file;
	struct proc_dir_entry *opponent_proc_file;

	password_file_contents = read_password_file();

	if (password_file_contents == NULL) {
		printk(KERN_ERR "failed to read password file");
		ttt_deinit();
		return -EFAULT;
	}

	parse_password_file(password_file_contents, strlen(password_file_contents));	

	user_proc_dirs = vmalloc(sizeof(struct proc_dir_entry*) * num_users);

	if (user_proc_dirs == NULL) {
		printk(KERN_ERR "ran out of memory for user_proc_dirs, tried to allocate %i spots \n", num_users);
		ttt_deinit();
		return -ENOMEM;
	}

	for (i = 0; i < num_users; i ++) {
		user_proc_dir = proc_mkdir(usernames[i], NULL);

		if (user_proc_dir == NULL) {
			printk(KERN_ERR "ran out of memory for proc dir %s \n", usernames[i]);
			ttt_deinit();
			return -ENOMEM;
		}

		user_proc_dirs[i] = user_proc_dir;
		game_proc_file = proc_create("game", 438, user_proc_dir, &game_fops);

		if (game_proc_file == NULL) {
			printk(KERN_ERR "ran out of memory for proc file game under proc dir %s \n", usernames[i]);
			ttt_deinit();
			return -ENOMEM;
		}

		opponent_proc_file = proc_create("opponent", 438, user_proc_dir, &opponent_fops);

		if (opponent_proc_file == NULL) {
			printk(KERN_ERR "ran out of memory for proc file opponent under proc dir %s \n", usernames[i]);
			ttt_deinit();
			return -ENOMEM;
		}
	}

	printk(KERN_INFO "tic-tac-toe module loaded.\n");
	return 0;
}

void ttt_deinit(void) {
	int i;

	for (i = 0; i < num_users; i ++) {
		remove_proc_entry("game", user_proc_dirs[i]);
		remove_proc_entry("opponent", user_proc_dirs[i]);
		remove_proc_entry(usernames[i], NULL);
		vfree(usernames[i]);
	}

	vfree(usernames);
	vfree(user_proc_dirs);

	printk(KERN_INFO "tic-tac-toe module unloaded.\n");
}

struct file_operations game_fops = {
	read: read_game,
	write: write_game
};

struct file_operations opponent_fops = {
	read: read_opponent,
	write: write_opponent
};

module_init(ttt_init);
module_exit(ttt_deinit);
