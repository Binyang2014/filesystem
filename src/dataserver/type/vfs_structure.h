/*
 * vfs_structure.h
 *
 * created on: 2015.1.31
 * author: Binyang
 *
 * this file define some structure to manipulate file system structures
 */
#ifndef _VFS_STRUCTURE_H_
#define _VFS_STRUCTURE_H_

#include <pthread.h>
#include "basic_structure.h"

struct super_block_operations
{
	pthread_mutex_t s_mutex;
	unsigned int (*get_blocks_count)();
	unsigned int (*get_free_block_count)();
	unsigned int (*get_filesystem_version)();
	unsigned int (*get_groups_conut)();
	time_t (*get_last_write_time)();
	unsigned int(*find_a_block_num)(unsigned int chunk_num);
	void (*print_sb_imf)();
};

struct file_operations
{
	int (*read)(unsigned int start_pos, unsigned int len);
	int (*open)(char *filename, int mode);
};

//only one copy in memory
struct datasetver_super_block
{
	struct super_block_operations *s_op;
};

//once open a file, this structure should be built
struct dataserver_file//I think we need a file structure to point opening files
{
	struct dataserver_file* head_list;//应该有个双向循环链表
	char file_name[256];
	unsigned int cur_off_side;
	unsigned int file_len;
	struct file_operations *f_op;
};
#endif
