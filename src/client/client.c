/*
 * client.c
 *
 *  Created on: 2015年7月20日
 *      Author: ron
 */

#include <stdarg.h>
#include <pthread.h>
#include <string.h>
#include "client.h"
#include "shmem.h"
#include "zmalloc.h"
#include "fifo_ipc.h"

static int fifo_fd;

static int open_file(const char *path, open_mode_t open_mode);
static int create_file(const char *path, open_mode_t open_mode, f_mode_t mode);
static void read_tmp_file();
static void read_persistent_file();
static void delete_tmp_file();
static void delete_persistent_file();
static void consistent_persistent_file();

static void merge_file();
static void read_disk_file();

static int open_file(const char *path, open_mode_t open_mode, int *fd)
{
	openfile_msg_t openfile_msg;
	file_ret_msg_t file_ret_msg;
	int path_len;

	openfile_msg.operation_code = FOPEN_OP;
	path_len = strlen(path);
	if(path_len > MAX_FILE_PATH)
		return FPATH_TOO_LONG;
	strcpy(openfile_msg.file_path, path);
	openfile_msg.open_mode = open_mode;

	if( write(fifo_fd, &openfile_msg, sizeof(openfile_msg_t)) < 0 )
	{
		sds_free(openfile_msg.file_path);
		return -1;
	}
	//get return code
	read(fifo_fd, &file_ret_msg, sizeof(file_ret_msg));
	*fd = file_ret_msg.fd;
	sds_free(openfile_msg.file_path);

	return file_ret_msg.ret_code;
}

static int create_file(const char *path, open_mode_t open_mode, f_mode_t mode,
		int *fd)
{
	createfile_msg_t create_msg;
	file_ret_msg_t file_ret_msg;
	int path_len;

	openfile_msg.operation_code = FCREATE_OP;
	path_len = strlen(path);
	if(path_len > MAX_FILE_PATH)
		return FPATH_TOO_LONG;
	strcpy(create_msg.file_path, path);
	create_msg.open_mode = open_mode;
	create_msg.mode = mode;

	if( write(fifo_fd, &create_msg, sizeof(createfile_msg_t)) < 0 )
	{
		sds_free(create_msg.file_path);
		return -1;
	}
	//get return code
	read(fifo_fd, &file_ret_msg, sizeof(file_ret_msg));
	*fd = file_ret_msg.fd;
	sds_free(create_msg.file_path);

	return file_ret_msg.ret_code;
}

//===========================================================================================

int init_client()
{
	fifo_fd = open_fifo(FIFO_PATH, O_RDWR);
}

void close_client()
{
	close_fifo(fifo_fd);
}

int f_open(const char *path, open_mode_t open_mode, ...)
{
	int fd;

	if( (open_mode & 0020) || (open_mode & 0040) )
	{
		va_list vl;
		f_mode_t mode;

		va_start(vl, open_mode);
		mode = va_arg(vl, f_mode_t);
		va_end(vl);
		if(create_file(path, open_mode, mode, &fd) != 0)
			return -1;
	}
	else
	{
		if(open_file(path, open_mode, &fd) != 0)
			return -1;
	}

	return fd;
}
