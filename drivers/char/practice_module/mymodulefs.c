#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <linux/uaccess.h>

#define MAX_BUF_SIZE (10485760) // 10MB

static char *shmem_buf;
size_t buf_size;
  
static ssize_t foo_show(struct kobject *kobj, struct kobj_attribute *attr,
			char *buf)
{
	return sprintf(buf, "Data size written on memory : %ld\n", buf_size);
}

static struct kobj_attribute foo_attribute =
	__ATTR_RO(foo);

ssize_t shmem_read(struct file *file, struct kobject *kobj, struct bin_attribute *attr,
		char *buf, loff_t off, size_t size)
{
	if(off > MAX_BUF_SIZE)
		return -EFAULT;
	memcpy(buf, shmem_buf, size);
	return size;
}

ssize_t shmem_write(struct file *file, struct kobject *kobj, struct bin_attribute *attr,
		 char *buf, loff_t off, size_t size)
{
	memcpy(shmem_buf, buf, size);
	buf_size = size;
	return size;
}

static vm_fault_t shmem_vm_fault(struct vm_fault *vmf)
{
	struct page *page = NULL;
	unsigned long offset = 0;
	void *ptr = NULL;

	if (vmf == NULL)
	{
		return VM_FAULT_SIGBUS;
	}
	offset = vmf->address - vmf->vma->vm_start;
	if (offset >= MAX_BUF_SIZE)
	{
		return VM_FAULT_SIGBUS;
	}
	ptr = shmem_buf + offset;
	page = vmalloc_to_page(ptr);
	if(!page)
		printk("fail vmalloc_to page");
	return vmf_insert_pfn(vmf->vma, vmf->address, page_to_pfn(page));  
}

static struct vm_operations_struct vma_ops = {
	.fault = shmem_vm_fault
};

static int shmem_mmap(struct file *file, struct kobject *kobj, struct bin_attribute *attr,
		struct vm_area_struct *vma)
{
	vma->vm_flags |= VM_IO;
	vma->vm_flags |= VM_PFNMAP;
	vma->vm_ops = &vma_ops;
	printk(KERN_ALERT "vm_start : %08lx, vm_end : %08lx, size : %ld\n",
		vma->vm_start, vma->vm_end, vma->vm_end-vma->vm_start);

	return 0;
}

static struct bin_attribute shmem_attribute = {
	.attr = { .name = "shmem", .mode = 0666 },
	.size = MAX_BUF_SIZE,
	.read = shmem_read,
	.write = shmem_write,
	.mmap = shmem_mmap,
}; 

static struct kobject *example_kobj;

static int mymodule_init(void)
{
	int ret = 0;

	shmem_buf = vmalloc(MAX_BUF_SIZE);
	if (!shmem_buf)
	{
		printk(KERN_ALERT "vmalloc is failed!\n");
		return ret;
	}
	memset(shmem_buf, 0, MAX_BUF_SIZE);

	example_kobj = kobject_create_and_add("myfs", kernel_kobj);
	if (!example_kobj)
		return -ENOMEM;

	ret = sysfs_create_bin_file(example_kobj, &shmem_attribute);
	ret = sysfs_create_file(example_kobj, &foo_attribute.attr);
	if (ret)
		kobject_put(example_kobj);

	printk(KERN_ALERT "Hello!\n");
	return ret;
}

static void mymodule_exit(void)
{
	printk(KERN_ALERT "Bye~\n");
	vfree(shmem_buf);
	kobject_put(example_kobj);
}

module_init(mymodule_init);
module_exit(mymodule_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nayeon");
MODULE_DESCRIPTION("WW02 Module practice");
