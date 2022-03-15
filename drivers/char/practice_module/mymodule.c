#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

static int mymodule_init(void)
{
	printk(KERN_ALERT "Hello!\n");
	return 0;
}

static void mymodule_exit(void)
{
	printk(KERN_ALERT "Bye~\n");
}

module_init(mymodule_init);
module_exit(mymodule_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nayeon");
MODULE_DESCRIPTION("WW02 Module practice");
