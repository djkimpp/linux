#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define SIZE 4096
#define KBYTE 1024

int main()
{
	int fd, cnt;
	char msg[SIZE*KBYTE]= {};
	cnt = 0;
	fd = open("./test.txt", O_CREAT | O_RDWR | O_TRUNC);

	while(1) {
		cnt++;
		write(fd, msg, SIZE*KBYTE);
		printf("Write : current size : %d KB\n", cnt*SIZE);
		sleep(1);
	}

	return 0;
}
