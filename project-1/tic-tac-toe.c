#include "linux/module.h"

int init()
{
	printk(KERN_INFO "Tic-Tac-Toe module loaded.\n");
	return 0;
}

void deinit()
{
	printk(KERN_INFO "Tic-Tac-Toe module unloaded.\n");
}

module_init(init);
module_exit(deinit);
