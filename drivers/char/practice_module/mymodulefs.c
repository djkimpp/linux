#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/kobject.h>
#include <linux/string.h>

static int foo;

/*
 * The "foo" file where a static variable is read from and written to.
 */
static ssize_t foo_show(struct kobject *kobj, struct kobj_attribute *attr,
			char *buf)
{
	return sprintf(buf, "%d\n", foo);
}

static ssize_t foo_store(struct kobject *kobj, struct kobj_attribute *attr,
			 const char *buf, size_t count)
{
	int ret;

	ret = kstrtoint(buf, 10, &foo);
	if (ret < 0)
		return ret;

	return count;
}

static struct kobj_attribute foo_attribute =
	__ATTR(foo, 0664, foo_show, foo_store);

static struct kobject *example_kobj;

static int mymodule_init(void)
{
	int ret;

	example_kobj = kobject_create_and_add("myfs", kernel_kobj);
	if (!example_kobj)
		return -ENOMEM;

	ret = sysfs_create_file(example_kobj, &foo_attribute.attr);
	if (ret)
		kobject_put(example_kobj);

	printk(KERN_ALERT "Hello!\n");
	return ret;
}

static void mymodule_exit(void)
{
	printk(KERN_ALERT "Bye~\n");

	kobject_put(example_kobj);
}

module_init(mymodule_init);
module_exit(mymodule_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nayeon");
MODULE_DESCRIPTION("WW02 Module practice");
