#ifndef FIFO_IPC_H_
#define FIFO_IPC_H_

#include <sys/stat.h>

#define FIFO_PATH_C_TO_S "client_c_to_s"
#define FIFO_PATH_S_TO_C "client_s_to_c"

int create_fifo(const char *path, mode_t mode);
int open_fifo(const char *path, int oflag);
void close_fifo(int fd);
void remove_fifo(const char *path);
#endif
