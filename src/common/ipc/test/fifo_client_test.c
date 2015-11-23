#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include "fifo_ipc.h"

char String[20];
int main()
{
	int fd;

	fd = open_fifo(FIFO_PATH_C_TO_S, O_RDWR);
	read(fd, String, 20);
	printf("string is %s\n", String);
	return 0;
}
