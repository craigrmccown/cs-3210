/**
 *  procfs2.c -  create a "file" in /proc
 *
 */

#include <linux/module.h>	/* Specifically, a module */
#include <linux/kernel.h>	/* We're doing kernel work */
#include <linux/proc_fs.h>	/* Necessary because we use the proc fs */
#include <asm/uaccess.h>	/* for copy_from_user */

#define PROCFS_MAX_SIZE		1024
#define PROCFS_NAME 		"buffer1k"

/**
 * This structure hold information about the /proc file
 *
 */
static struct proc_dir_entry *proc_entry;

/**
 * The buffer used to store character for this module
 *
 */
static char procfs_buffer[PROCFS_MAX_SIZE];

/**
 * The size of the buffer
 *
 */
static unsigned long procfs_buffer_size = 0;

/** 
 * called when file is closed
 *
 */
static int procfile_close(struct inode* inode, struct file* file) {
	int ret = 0;

	printk(KERN_INFO "procfile_close (/proc/%s) called\n", PROCFS_NAME);

	return ret;
}

/**
 * defines file operations such as read/write/open/close callbacks
 *
 */
static const struct file_operations file_ops = {
	.read = procfile_close,
};

/**
 * initializes module
 *
 */
int init_module()
{
	/* create the /proc file */
	proc_entry = proc_create(PROCFS_NAME, 0644, NULL, &file_ops);

	if (proc_entry == NULL) {
		remove_proc_entry(PROCFS_NAME, &proc_root);
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n",
			PROCFS_NAME);
		return -ENOMEM;
	}

	printk(KERN_INFO "/proc/%s created\n", PROCFS_NAME);	
	return 0;	/* everything is ok */
}

/**
 * cleans up when module is unloaded
 *
 */
void cleanup_module()
{
	remove_proc_entry(PROCFS_NAME, &proc_root);
	printk(KERN_INFO "/proc/%s removed\n", PROCFS_NAME);
}
