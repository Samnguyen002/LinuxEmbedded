#include <sys/stat.h>
#include <stdio.h>
#include <sys/file.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main(void) 
{
	int fd;
	char text[16] = {0};

	sprintf(text,"hello word\n");
	fd = open("./text.txt", O_RDWR);

	write(fd, text, sizeof(text)-1);
	

	if(flock(fd,LOCK_EX) == -1) {
		printf("can not set read lock\n");
	} else {
		printf("set read lock\n");
	}
    
	while(1) {
		sleep(1);
	}
	close(fd);

	return 0;
}
