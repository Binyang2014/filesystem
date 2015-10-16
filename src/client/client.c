/*
 * client.c
 *
 *  Created on: 2015年7月20日
 *      Author: ron
 */

#include <stdarg.h>
#include "client.h"
#include "shmem.h"
#include "zmalloc.h"

static void create_tmp_file();
static void create_persistent_file();
static void read_tmp_file();
static void read_persistent_file();
static void delete_tmp_file();
static void delete_persistent_file();
static void consistent_persistent_file();

static void merge_file();
static void read_disk_file();

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
	//send message to client server
	if( (open_mode & 0020) || (open_mode & 0040) )
	{
		va_list vl;
		f_mode_t mode;

		va_start(vl, open_mode);
		mode = va_arg(vl, f_mode_t);
		va_end(vl);
		//send message to client server
	}

	return file;
}
