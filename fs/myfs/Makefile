obj-m += myfs.o
myfs-objs := module.o inode.o dir.o file.o bitmap.o

KDIR := /lib/modules/$(shell uname -r)/build

all:
	make -C $(KDIR) M=$(PWD) modules
clean:
	make -C $(KDIR) M=$(PWD) clean
