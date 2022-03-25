#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>

#define BUF_SIZE 4096

char write_buf[BUF_SIZE];
char read_buf[BUF_SIZE];

int writeData(void) {
	int fd, ret;

	fd = open("/sys/kernel/myfs/shmem", O_WRONLY, S_IRUSR | S_IWUSR);
	if(!fd) {
		printf("File open failed!\n");
		return 0;
	}
	
	for(int idx=0; idx<BUF_SIZE; idx++) {
		write_buf[idx] = (idx%10) + '0';
	}
	
	ret = write(fd, write_buf, BUF_SIZE);

	close(fd);
	return ret;
}

int readData(void) {
	int fd, ret;
	fd = open("/sys/kernel/myfs/shmem", O_RDONLY);
	if(!fd) {
		printf("File open failed!\n");
		return 0;
	}
	
	ret = read(fd, read_buf, BUF_SIZE);

	close(fd);
	return ret;
}

int main(int argc, char *argv[])
{
	int ret = writeData();
	if(ret < 0) {
		printf("write error !");
		return 0;
	}

	ret = readData();
	if (ret < 0) {
		printf("Read error!\n");
		return 0;
	}

	int result = memcmp(read_buf, write_buf, BUF_SIZE);
	if(result == 0) {
		printf("memcmp success!\n");
	}
	else
	{
		printf("memcmp failed!\n");
	}

	return 0;
}
