#include <stdio.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <unistd.h>
#include<fcntl.h>

int main(void) 
{
	int fd;
	char buf[16] = {0};

	fd = open("./text.txt",O_RDWR);
	

	if(flock(fd,LOCK_EX) == -1) {
		printf("can not get write lock\n");
	}

	printf("hello processB");

	close(fd);

	return 0;
}
