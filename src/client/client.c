/*
 * client.c
 *
 *  Created on: 2015年7月20日
 *      Author: ron
 */

#include <stdarg.h>
#include <pthread.h>
#include <string.h>
#include <fcntl.h>
#include "log.h"
#include "client.h"
#include "zmalloc.h"
#include "client_server.h"

static int open_file(const char *path, open_mode_t open_mode, int *fd);
static int create_file(const char *path, open_mode_t open_mode, f_mode_t mode, int *fd);

static int open_file(const char *path, open_mode_t open_mode, int *fd)
{
	openfile_msg_t openfile_msg;
	int path_len, ret_code;

	openfile_msg.operation_code = FOPEN_OP;
	path_len = strlen(path);
	if(path_len > MAX_FILE_PATH)
	{
		return FPATH_TOO_LONG;
	}
	strcpy(openfile_msg.file_path, path);
	openfile_msg.open_mode = open_mode;

	ret_code = fs_open(&openfile_msg, fd);
	log_write(LOG_DEBUG, "the fd is %d", *fd);

	return ret_code;
}

static int create_file(const char *path, open_mode_t open_mode, f_mode_t mode, int *fd)
{
	createfile_msg_t createfile_msg;
	int path_len, ret_code;

	createfile_msg.operation_code = FCREATE_OP;
	path_len = strlen(path);
	if(path_len > MAX_FILE_PATH)
	{
		return FPATH_TOO_LONG;
	}
	strcpy(createfile_msg.file_path, path);
	createfile_msg.open_mode = open_mode;
	createfile_msg.mode = mode;

	//call client server functions
	ret_code = fs_create(&createfile_msg, fd);
	log_write(LOG_DEBUG, "the fd is %d", *fd);

	return ret_code;
}

//===========================================================================================

int f_open(const char *path, open_mode_t open_mode, ...)
{
	int fd;

	if( (open_mode & 0020) || (open_mode & 0040) )
	{
		va_list vl;
		f_mode_t mode;

		va_start(vl, open_mode);
		mode = va_arg(vl, int);
		va_end(vl);
		if(create_file(path, open_mode, mode, &fd) != 0)
		{
			return -1;
		}
	}
	else
	{
		if(open_file(path, open_mode, &fd) != 0)
		{
			return -1;
		}
	}

	return fd;
}

int f_close(int fd)
{
	closefile_msg_t closefile_msg;
	int ret_code;

	closefile_msg.operation_code = FCLOSE_OP;
	closefile_msg.fd = fd;
	ret_code = fs_close(&closefile_msg);
	if(ret_code != 0)
		return -1;
	else
		return 0;
}

//This function maybe not return actually read charictors
//should be modified later
ssize_t f_read(int fd, void *buf, size_t nbytes)
{
	readfile_msg_t readfile_msg;
	ssize_t ret_code;

	readfile_msg.operation_code = FREAD_OP;
	readfile_msg.data_len = nbytes;
	readfile_msg.fd = fd;

	ret_code = fs_read(&readfile_msg, buf);
	if(ret_code != 0)
		return -1;
	else
		return nbytes;
}

//This function maybe not return actually append charictors
//should be modified later
ssize_t f_append(int fd, const void *buf, size_t nbytes)
{
	appendfile_msg_t appendfile_msg;
	int ret_code;

	appendfile_msg.operation_code = FAPPEND_OP;
	appendfile_msg.data_len = nbytes;
	appendfile_msg.fd = fd;

	ret_code = fs_append(&appendfile_msg, buf);
	if(ret_code != 0)
		return -1;
	else
		return nbytes;
}

int f_remove(const char *pathname)
{
	removefile_msg_t removefile_msg;
	int ret_code;

	removefile_msg.operation_code = FREMOVE_OP;
	strcpy(removefile_msg.file_path, pathname);

	ret_code = fs_remove(&removefile_msg);
	if(ret_code != 0)
		return -1;
	else
		return 0;
}
