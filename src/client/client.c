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
#include "fifo_ipc.h"

static int fifo_wfd;
static int fifo_rfd;

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

	if( write(fifo_wfd, &openfile_msg, sizeof(openfile_msg_t)) < 0 )
		return -1;
	//get return code
	read(fifo_rfd, &file_ret_msg, sizeof(file_ret_msg));
	*fd = file_ret_msg.fd;

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

	if( write(fifo_wfd, &createfile_msg, sizeof(createfile_msg_t)) < 0 )
		return -1;
	//get return code
	read(fifo_rfd, &file_ret_msg, sizeof(file_ret_msg));
	log_write(LOG_DEBUG, "the fd is %d", file_ret_msg.fd);
	*fd = file_ret_msg.fd;

	return file_ret_msg.ret_code;
}

//===========================================================================================

int init_client()
{
	fifo_wfd = open_fifo(FIFO_PATH_C_TO_S, O_RDWR);
	fifo_rfd = open_fifo(FIFO_PATH_S_TO_C, O_RDWR);
	if(fifo_wfd < 0 || fifo_rfd < 0)
	{
		log_write(LOG_ERR, "could not connect to client server");
		return -1;
	}
	else
		log_write(LOG_INFO, "connect to server successfully");
	return 0;
}

void close_client()
{
	close_fifo(fifo_wfd);
	close_fifo(fifo_rfd);
}

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
	closefile_msg_t closefile_msg;
	file_ret_msg_t file_ret_msg;

	closefile_msg.operation_code = FCLOSE_OP;
	closefile_msg.fd = fd;
	write(fifo_wfd, &closefile_msg, sizeof(closefile_msg_t));
	read(fifo_rfd, &file_ret_msg, sizeof(file_ret_msg_t));
	return file_ret_msg.ret_code;
}

ssize_t f_read(int fd, void *buf, size_t nbytes)
{
	readfile_msg_t readfile_msg;
	file_ret_msg_t file_ret_msg;
	int ret;

	readfile_msg.operation_code = FREAD_OP;
	readfile_msg.data_len = nbytes;
	readfile_msg.fd = fd;

	write(fifo_wfd, &readfile_msg, sizeof(readfile_msg_t));
	read(fifo_rfd, buf, nbytes);
	write(fifo_wfd, &ret, sizeof(int));
	read(fifo_rfd, &file_ret_msg, sizeof(file_ret_msg_t));
	return file_ret_msg.ret_code;
}

ssize_t f_append(int fd, const void *buf, size_t nbytes)
{
	appendfile_msg_t appendfile_msg;
	file_ret_msg_t file_ret_msg;
	int ret;

	appendfile_msg.operation_code = FAPPEND_OP;
	appendfile_msg.data_len = nbytes;
	appendfile_msg.fd = fd;

	write(fifo_wfd, &appendfile_msg, sizeof(appendfile_msg_t));
	read(fifo_rfd, &ret, sizeof(int));
	write(fifo_wfd, buf, nbytes);
	read(fifo_rfd, &file_ret_msg, sizeof(file_ret_msg_t));
	return file_ret_msg.ret_code;
}

int f_remove(const char *pathname)
{
	removefile_msg_t removefile_msg;
	file_ret_msg_t file_ret_msg;

	removefile_msg.operation_code = FREMOVE_OP;
	strcpy(removefile_msg.file_path, pathname);

	write(fifo_wfd, &removefile_msg, sizeof(removefile_msg_t));
	read(fifo_rfd, &file_ret_msg, sizeof(file_ret_msg_t));
	return file_ret_msg.ret_code;
}
