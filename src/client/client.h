/*
 * client.h
 *
 *  Created on: 2015年7月7日
 *      Author: ron
 * modified by: Binyang
 * This file define some APIs witch user can use to communicate with our
 * framwork.
 * before run any APIs you should run init_client first.
 */

#ifndef CLIENT_H_
#define CLIENT_H_

#include <unistd.h>
#include "global.h"
#include "client_struct.h"

int init_client();
int f_open(const char *path, enum open_mode, ...);
int f_close(opened_file_t *file);
ssize_t f_read(opened_file_t *file, void *buf, size_t nbytes);
ssize_t f_write(opened_file_t *file, const void *buf, size_t nbytes);
int f_remove(const char *pathname);
int f_mkdir(const char *pathname, f_mode_t mode);
int f_rmdir(const char *pathname);

#endif /* CLIENT_H_ */
