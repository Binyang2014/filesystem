#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "fifo_ipc.h"
#include "log.h"

int create_fifo(const char *path, mode_t mode)
{
	int ret;

	ret = mkfifo(path, mode);
	if(ret != 0)
		log_write(LOG_DEBUG, "error when create fifo");
	return ret;
}

int open_fifo(const char *path, int oflag)
{
	int fd;

	fd = open(path, oflag);
	return fd;
}

void close_fifo(int fd)
{
	close(fd);
}

void remove_fifo(const char *path)
{
	remove(path);
}
