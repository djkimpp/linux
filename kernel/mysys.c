#include <linux/kernel.h>
#include <linux/syscalls.h>

SYSCALL_DEFINE1(my_syscall, int, num)
{
	printk("my_syscall is called\n");
	printk("my_syscall : %d\n", num);
	num += 1;

	return num;
}
