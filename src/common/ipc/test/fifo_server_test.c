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
	create_fifo(FIFO_PATH_C_TO_S, S_IRWXU);
	fd = open_fifo(FIFO_PATH_C_TO_S, O_RDWR);

	strcpy(String, FIFO_PATH_C_TO_S);
	write(fd, String, 20);
	sleep(20);
	close_fifo(fd);
	remove_fifo(FIFO_PATH_C_TO_S);

	return 0;
}
