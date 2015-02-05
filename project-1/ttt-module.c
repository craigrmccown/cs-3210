#include "ttt-module.h"

ssize_t read_game(struct file *f, char *buffer, size_t count, loff_t *offset) {
	struct ttt_game *game;
	char game_board[18];
	int i, move, bytes_read;
	char *player_name;

	printk(KERN_INFO "read_game called \n");

	player_name = "hello";
	game = find_game_by_username(player_name);

	if (game == NULL) {
		if (copy_to_user(buffer, "you are not currently playing a game \n", 38)) {
			return -EFAULT;
		} else {
			bytes_read = 38 - *offset;
			*offset = 38;
			return bytes_read;
		}
	}

	for (i = 0; i < 18; i ++) {
		if (i % 2 != 0) {
			if ((i - 5) % 6 == 0) {
				game_board[i] = '\n';
			} else {
				game_board[i] = '|';
			}
		} else {
			game_board[i] = '_';	
		}
	}

	for (i = 0; i < 5; i ++) {
		move = (game -> player1_moves)[i];
		
		if (move != -1) {
			game_board[move * 2] = 'X';
		}

		move = game -> player2_moves[i];
		
		if (move != -1) {
			game_board[move * 2] = 'O';
		}
	}

	printk(KERN_INFO "game board: %s \n", game_board); 
	if (copy_to_user(buffer, game_board, 18)) {
		return -EFAULT;
	} else {
		bytes_read = 18 - *offset;
		*offset = 18;
		return bytes_read;
	}
}

ssize_t write_game(struct file *f, const char *buffer, size_t count, loff_t *offset) {
	char input[1];
	long move[1];
	char *player_name;
	struct ttt_game *game;
	int valid_input;

	printk(KERN_INFO "write_game called \n");

	player_name = "hello";
	game = find_game_by_username(player_name);

	if (game == NULL) {
		printk(KERN_ERR "player %s is not playing a game \n", player_name);
		return -EFAULT;
	}

	if (strcmp(player_name, game -> next_player) != 0) {
		printk(KERN_ERR "player %s tried to move, but it is not their turn \n", player_name);
		return -EFAULT;
	}

	if (copy_from_user(input, buffer, 1)) {
		printk(KERN_ERR "failed to copy input from user space \n");
		return -EFAULT;
	}

	valid_input = kstrtol(input, 10, move);

	if (valid_input != 0 || *move > 8 || *move < 0) {
		printk(KERN_ERR "invalid input \n");
		return -EFAULT;
	}	
	
	if (strcmp(player_name, game -> player1) == 0) {
		game -> player1_moves[game -> player1_num_moves] = *move;
		game -> player1_num_moves ++;
		game -> next_player = game -> player2;
	} else {
		game -> player2_moves[game -> player2_num_moves] = *move;
		game -> player2_num_moves ++;
		game -> next_player = game -> player1;
	}
	
	return count;
}

/*
ssize_t read_opponent(struct file *f, char *buffer, size_t count, loff_t *offset) {
	char* message;

	proc_buffer_len = strlen(proc_buffer);

	if (*offset + count > proc_buffer_len) {
		count = proc_buffer_len - *offset;
	}

	if ((not_copied=copy_to_user(buffer + *offset, proc_buffer, count))) {
		count = -EFAULT;
	} 
	next_offset = *offset + (count - not_copied);
	*offset = next_offset;

	return count - not_copied;
}
*/

ssize_t write_opponent(struct file *f, const char *buffer, size_t count, loff_t *offset) {
	char *opponent_name;
	int buffer_len, i;
	char *player_name = "haah";

	buffer_len = strlen(buffer);
	opponent_name = vmalloc(sizeof(char) * buffer_len);

	if (copy_from_user(opponent_name, buffer, buffer_len)) {
		printk(KERN_ERR "failed to copy opponent write data from user space \n");
		return -EFAULT;
	}

	if (find_game_by_username(player_name) != NULL) {
		printk(KERN_ERR "player %s is already playing a game \n", player_name);
		return -EFAULT;
	}

	opponent_name = sanitize_user(opponent_name);

	if (opponent_name == NULL) {
		printk(KERN_ERR "opponent does not exist \n");
		return -EFAULT;
	}

	if (num_games == MAX_GAMES) {
		printk(KERN_ERR "maximum number of games exceeded \n");
		return -EFAULT;
	}

	games[num_games] = vmalloc(sizeof(struct ttt_game));

	if (games[num_games] == NULL) {
		printk(KERN_ERR "insufficient memory to initialize game \n");
		return -ENOMEM;
	}

	games[num_games] -> player1 = player_name;	
	games[num_games] -> player2 = opponent_name;	
	games[num_games] -> player1_num_moves = 0;
	games[num_games] -> player2_num_moves = 0;
	games[num_games] -> next_player = player_name;	

	for (i = 0; i < 5; i ++) {
		games[num_games] -> player1_moves[i] = -1;
		games[num_games] -> player2_moves[i] = -1;
	} 

	num_games ++;
	return count;
}

struct ttt_game *find_game_by_username(char *username) {
	int i;

	for (i = 0; i < num_games; i ++) {
		if (
			strcmp(games[i] -> player1, username) == 0 ||
			strcmp(games[i] -> player2, username) == 0
		) {
			return games[i];
		}
	}

	return NULL;
}

char *sanitize_user(char* username) {
	int i;

	for (i = 0; i < num_users; i ++) {
		if (strcmp(usernames[i], username) == 0) {
			return usernames[i];
		}
	}

	return NULL;
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
	int i, line_count, current_char, colon_count, j;
	char *username;
	char *uid;

	line_count = 0;
	colon_count = 0;
	j = 0;

	for (i = 0; i < len; i ++) {
		if (password_file_contents[i] == '\n') {
			line_count = line_count + 1;
		}
	}

	usernames = vmalloc(sizeof(char*) * line_count);
	username = vmalloc(sizeof(char) * 25);
	uids = vmalloc(sizeof(char*) * line_count);
	uid = vmalloc(sizeof(char) * 25);
	num_users = 0;
	current_char = 0;

	for (i = 0; i < len; i ++) {
		if (colon_count == 4) {
			if (password_file_contents[i] == ':') {
			colon_count++;
		}
			uid[j] = password_file_contents[i];
			j++;
		}
		else if (current_char == -1) {
			if (password_file_contents[i] == '\n') {
				usernames[num_users] = username;
				uids[num_users] = uid;
				username = vmalloc(sizeof(char) * 25);
				uid = vmalloc(sizeof(char) * 25);
				current_char = 0;
				colon_count = 0;
				j = 0;
				num_users = num_users + 1;
			}
		} else if (password_file_contents[i] == ':') {
			username[current_char] = '\0';
			current_char = -1;
			colon_count++;
		}  else {
			username[current_char] = password_file_contents[i];
			current_char = current_char + 1;
		}
	}

	return line_count;
}

/*
char *get_player_name(char* uid) {
	int i;
}
*/

int ttt_init(void) {
	int i;
	char *password_file_contents;
	struct proc_dir_entry *user_proc_dir;
	struct proc_dir_entry *game_proc_file;
	struct proc_dir_entry *opponent_proc_file;

	password_file_contents = read_password_file();

	if (password_file_contents == NULL) {
		printk(KERN_ERR "failed to read password file \n");
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

 	games = vmalloc(sizeof(struct ttt_game *) * MAX_GAMES);

	if (games == NULL) {
		ttt_deinit();
		return -ENOMEM;
	}

	num_games = 0;

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
		vfree(uids[i]);
	}

	vfree(usernames);
	vfree(uids);
	vfree(user_proc_dirs);
	vfree(games);

	printk(KERN_INFO "tic-tac-toe module unloaded.\n");
}

struct file_operations game_fops = {
	read: read_game,
	write: write_game
};

struct file_operations opponent_fops = {
	// read: read_opponent,
	write: write_opponent
};

module_init(ttt_init);
module_exit(ttt_deinit);
