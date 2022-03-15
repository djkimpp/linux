#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	int ret, num;
	num = atoi(argv[1]);
	printf("Before the syscall is called, user input is %d\n", num);
	ret = syscall(451, num);
	printf("After the syscall is called, return value is %d\n", ret);
	return 0;
}
