#include "ttt-game.h"

/*
char proc_buffer[PROC_BUFFER_SIZE];
char password_file_buffer[PASSWORD_BUFFER_SIZE];
int num_users;
char **usernames;
struct proc_dir_entry **user_proc_dirs;
struct file_operations proc_fops;

void initialize(int x) {
	num_users = x;
}
*/

void print_num_users(void) {
	printk(KERN_INFO "number of users: %i \n", num_users);
}
