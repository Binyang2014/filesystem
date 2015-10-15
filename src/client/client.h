/*
 * client.h
 *
 *  Created on: 2015年7月7日
 *      Author: ron
 * modified by: Binyang
 * This file define some APIs witch user can use to communicate with our
 * framwork.
 */

#ifndef CLIENT_H_
#define CLIENT_H_

#include <unistd.h>
#include "global.h"
#include "client_struct.h"

#define RDONLY 0001
#define WRONLY 0002
#define RDWR 0004
#define APPEND 0010
#define CREATE_TEMP 0020
#define CREATE_PERSIST 0040

opened_file_t *f_open(const char *path, enum open_mode, ...);
int f_close(opened_file_t *file);
ssize_t f_read(opened_file_t *file, void *buf, size_t nbytes);
ssize_t f_write(opened_file_t *file, const void *buf, size_t nbytes);
int remove(const char *pathname);
int mkdir(const char *pathname, f_mode_t mode);
int rmdir(const char *pathname);

typedef unsigned short open_mode_t;
#endif /* CLIENT_H_ */
