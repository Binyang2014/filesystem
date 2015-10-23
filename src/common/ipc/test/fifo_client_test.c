#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include "fifo_ipc.h"

char String[20];
int main()
{
	int fd;

	fd = open_fifo(FIFO_PATH, O_RDWR);
	read(fd, String, 20);
	printf("string is %s", String);
	return 0;
}
