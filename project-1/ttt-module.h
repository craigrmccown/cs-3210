#ifndef TTT_MODULE_H_
#define TTT_MODULE_H_

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/cred.h>
#include <linux/init.h>
#include <linux/sched.h>


#define PROC_BUFFER_SIZE 256
#define PASSWORD_BUFFER_SIZE 2048
#define MAX_GAMES 10

struct ttt_game {
	char* player1;
	char* player2;
	int player1_num_moves;
	int player2_num_moves;
	int player1_moves[5];
	int player2_moves[5];
	char* next_player;
};

ssize_t read_game(struct file *f, char *buffer, size_t count, loff_t *offset);
ssize_t write_game(struct file *f, const char *buffer, size_t count, loff_t *offset);
ssize_t read_opponent(struct file *f, char *buffer, size_t count, loff_t *offset);
ssize_t write_opponent(struct file *f, const char *buffer, size_t count, loff_t *offset);
struct ttt_game *find_game_by_username(char *username);
char *get_player_name(int uid);
char *sanitize_user(char* username);
char *read_password_file(void);
int parse_password_file(char *password_file_contents, int len);
int ttt_init(void);
void ttt_deinit(void);

char proc_buffer[PROC_BUFFER_SIZE];
char password_file_buffer[PASSWORD_BUFFER_SIZE];
int num_users;
int num_games;
char **usernames;
long *uids;
struct proc_dir_entry **user_proc_dirs;
struct ttt_game **games;
struct file_operations game_fops;
struct file_operations opponent_fops;

#endif
