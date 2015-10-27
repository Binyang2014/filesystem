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
#include "zmalloc.h"
#include "fifo_ipc.h"

static int fifo_fd;

static int open_file(const char *path, open_mode_t open_mode, int *fd);
static int create_file(const char *path, open_mode_t open_mode, f_mode_t mode,
		int *fd);
/*static void read_tmp_file();
static void read_persistent_file();
static void delete_tmp_file();
static void delete_persistent_file();
static void consistent_persistent_file();

static void merge_file();
static void read_disk_file();*/

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
	createfile_msg_t createfile_msg;
	file_ret_msg_t file_ret_msg;
	int path_len;

	createfile_msg.operation_code = FCREATE_OP;
	path_len = strlen(path);
	if(path_len > MAX_FILE_PATH)
		return FPATH_TOO_LONG;
	strcpy(createfile_msg.file_path, path);
	createfile_msg.open_mode = open_mode;
	createfile_msg.mode = mode;

	if( write(fifo_fd, &createfile_msg, sizeof(createfile_msg_t)) < 0 )
	{
		sds_free(createfile_msg.file_path);
		return -1;
	}
	//get return code
	read(fifo_fd, &file_ret_msg, sizeof(file_ret_msg));
	*fd = file_ret_msg.fd;
	sds_free(createfile_msg.file_path);

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

int f_close(int fd)
{
	int fd;
	closefile_msg_t closefile_msg;
	file_ret_msg_t file_ret_msg;

	closefile_msg.operation_code = FCLOSE_OP;
	closefile_msg.fd = fd;
	write(fifo_fd, &closefile_msg, sizeof(closefile_msg_t));
	read(fifo_fd, &file_ret_msg, sizeof(file_ret_msg_t));
	return file_ret_msg.ret_code;
}

int f_read(int fd, void *buf, size_t nbytes)
{
	readfile_msg_t readfile_msg;
	file_ret_msg_t file_ret_msg;

	readfile_msg.operation_code = FREAD_OP;
	readfile_msg.data_len = nbytes;
	readfile_msg.fd = fd;

	write(fifo_fd, &readfile_msg, sizeof(readfile_msg_t));
	read(fifo_fd, &file_ret_msg, sizeof(file_ret_msg_t));
	return file_ret_msg.ret_code;
}

int f_append(int fd, void *buf, size_t nbytes)
{
	appendfile_msg_t appendfile_msg;
	file_ret_msg_t file_ret_msg;

	appendfile_msg.operation_code = FAPPEND_OP;
	appendfile_msg.data_len = nbytes;
	appendfile_msg.fd = fd;

	write(fifo_fd, &appendfile_msg, sizeof(appendfile_msg_t));
	read(fifo_fd, &file_ret_msg, sizeof(file_ret_msg_t));
	return file_ret_msg.ret_code;
}

int f_remove(const char *pathname)
{
	removefile_msg_t removefile_msg;
	file_ret_msg_t file_ret_msg;

	removefile_msg.operation_code = FREMOVE_OP;
	strcpy(removefile_msg.file_path, pathname);

	write(fifo_fd, &removefile, sizeof(removefile_msg_t));
	read(fifo_fd, &file_ret_msg, sizeof(file_ret_msg_t));
	return file_ret_msg.ret_code;
}
