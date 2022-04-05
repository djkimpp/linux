#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>

#define BUF_SIZE 5000000

void writeData(void) {
	int fd = open("/sys/kernel/myfs/shmem", O_RDWR);
	char *mapped = NULL;
	mapped = (char *)mmap(0, BUF_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(mapped==NULL) {
		printf("mmap error!\n");
		return;
	}
	else
	{
		printf("mmap success! address : %p\n", mapped);
		for(int idx=0; idx<BUF_SIZE; idx++) {
			mapped[idx] = (idx%10) + '0';
		}
	}
	close(fd);
}

void readData(void) {
	int fd = open("/sys/kernel/myfs/shmem", O_RDWR);
	char *mapped = NULL;
	mapped = (char *)mmap(0, BUF_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(mapped==NULL) {
		printf("mmap error!\n");
		return;
	}
	else
	{
		printf("mmap success! address : %p\n", mapped);
		for(int idx=0; idx<BUF_SIZE; idx++) {
			if(mapped[idx] != (idx%10) + '0')
			{
				printf("FAIL!\n");
				return;
			}
		}
		printf("SUCCESS!\n");
	}
	close(fd);
}

int main(int argc, char *argv[])
{
	writeData();

	readData();

	return 0;
}
