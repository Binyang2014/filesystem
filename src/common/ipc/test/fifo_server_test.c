#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "fifo_ipc.h"
#include "log.h"

char String[20];
int main()
{
	int fd;

	log_init("", LOG_DEBUG);
	create_fifo(FIFO_PATH, S_IRWXU);
	fd = open_fifo(FIFO_PATH, O_RDWR);

	strcpy(String, FIFO_PATH);
	write(fd, String, 20);
	sleep(20);
	close_fifo(fd);
	remove_fifo(FIFO_PATH);

	return 0;
}
