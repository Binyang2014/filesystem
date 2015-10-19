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

static shmem_t *shmem = NULL;
static pthread_mutex_t *shm_mutex = NULL;

static int open_file(const char *path, open_mode_t open_mode);
static int create_file(const char *path, open_mode_t open_mode, f_mode_t mode);
static void read_tmp_file();
static void read_persistent_file();
static void delete_tmp_file();
static void delete_persistent_file();
static void consistent_persistent_file();

static void merge_file();
static void read_disk_file();

static int open_file(const char *path, open_mode_t open_mode)
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

	pthread_mutex_lock(shm_mutex);
	if( send_to_shm(shmem, &openfile_msg, sizeof(openfile_msg_t)) != 0 )
	{
		sds_free(openfile_msg.file_path);
		pthread_mutex_unlock(shm_mutex);
		return -1;
	}
	//get return code
	recv_from_shm(shmem, &file_ret_msg);
	pthread_mutex_unlock(shm_mutex);
	sds_free(openfile_msg.file_path);
	return file_ret_msg.ret_code;
}

static int create_file(const char *path, open_mode_t open_mode, f_mode_t mode)
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

	pthread_mutex_lock(shm_mutex);
	if( send_to_shm(shmem, &create_msg, sizeof(createfile_msg_t)) != 0 )
	{
		sds_free(create_msg.file_path);
		pthread_mutex_unlock(shm_mutex);
		return -1;
	}
	//get return code
	recv_from_shm(shmem, &file_ret_msg);
	pthread_mutex_unlock(shm_mutex);
	sds_free(create_msg.file_path);
	return file_ret_msg.ret_code;
}

int init_client()
{
	shmem = get_shm(COMMON_KEY, SHM_UREAD | SHM_UWRITE);
	shm_mutex = zmalloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(shm_mutex);
	//attach memory to this process address space
	attach_shm(shmem);
}

void close_client()
{
	detach_shm(shmem);
	shm_free(shmem);
}

opened_file_t *f_open(const char *path, open_mode_t open_mode, ...)
{
	opened_file_t *file;

	file = zmalloc(sizeof(opened_file_t));
	if(!file)
		return NULL;
	switch(open_mode & 017)
	{
		case RDONLY:
			file->f_stat = F_RDONLY;
			break;

		case WRONLY:
			file->f_stat = F_WRONLY;
			break;

		case RDWR:
			file->f_stat = F_RDWD;
			break;

		case APPEND:
			file->f_stat = F_APPEND;
			break;

		default:
			zfree(file);
			return NULL;
	}
	if( (open_mode & 0020) || (open_mode & 0040) )
	{
		va_list vl;
		f_mode_t mode;

		va_start(vl, open_mode);
		mode = va_arg(vl, f_mode_t);
		va_end(vl);
		if(create_file(path, open_mode, mode) != 0)
		{
			zfree(file);
			return NULL;
		}
	}
	else
	{
		if(open_file(path, open_mode) != 0)
		{
			zfree(file);
			return NULL;
		}
	}

	return file;
}
