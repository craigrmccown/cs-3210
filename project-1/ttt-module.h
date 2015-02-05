#ifndef TTT_MODULE_H_
#define TTT_MODULE_H_

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>

#define PROC_BUFFER_SIZE 256
#define PASSWORD_BUFFER_SIZE 2048

ssize_t read_proc(struct file *f, char *buffer, size_t count, loff_t *offset);
ssize_t write_proc(struct file *f, const char *buffer, size_t count, loff_t *offset);
char *read_password_file(void);
int parse_password_file(char *password_file_contents, int len);
int ttt_init(void);
void ttt_deinit(void);

char proc_buffer[PROC_BUFFER_SIZE];
char password_file_buffer[PASSWORD_BUFFER_SIZE];
int num_users;
char **usernames;
struct proc_dir_entry **user_proc_dirs;
struct file_operations proc_fops;

#endif
