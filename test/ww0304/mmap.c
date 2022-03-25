#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>

#define BUF_SIZE 10485760 // 10MB

int main(int argc, char *argv[])
{
	int fd = open("/sys/kernel/myfs/shmem", O_RDWR);
	char *mapped = NULL;
	mapped = (char *)mmap(0, BUF_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(mapped==NULL) {
		printf("mmap error!\n");
		return 0;
	}
	else
	{
		printf("mmap success! address : %p\n", mapped);
		mapped[0] = 'd';
		printf("mapped data :%s\n", mapped);
	}
	close(fd);
	return 0;
}
