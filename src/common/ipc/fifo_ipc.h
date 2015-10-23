#ifndef FIFO_IPC_H_
#define FIFO_IPC_H_

#include <sys/stat.h>

#define FIFO_PATH "/tmp/client_c_to_s"

int create_fifo(const char *path, mode_t mode);
int open_fifo(const char *path, int oflag);
void close_fifo(int fd);
void remove_fifo(const char *path);
#endif
