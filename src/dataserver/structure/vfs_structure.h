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

struct list_head
{
	struct list_head *pre;
	struct list_head *next;
};

struct super_block_operations
{
	unsigned int (*get_blocks_count)(struct super_block*);
	unsigned int (*get_free_block_count)(struct super_block*);
	unsigned int (*get_filesystem_version)(struct super_block*);
	unsigned int (*get_groups_conut)(struct super_block*);
	time_t (*get_last_write_time)(struct super_block*);
	unsigned int(*find_a_block_num)(unsigned int chunk_num);
	void (*find_serials_blocks)(struct super_block*, unsigned int* chunks_arr, unsigned int* blocks_arr);
	int (*alloc_blocks)(struct super_block*, int blocks_count, unsigned int *chunks_arr, unsigned int *blocks_arr);//用给定的chunk号数组分配适量的blocks
	int (*free_blocks)(struct super_block*, int blocks_count, unsigned int *chunks_arr);
	void (*print_sb_imf)(struct super_block*);
	//set chunk and block
	//clear chunk and block
};

//enum open_mode
//{
//	READ = 0x01,
//	WRITE = 0x10,
//	APPEND = 0x100,
//	TEXT = 0x1000,
//	BINARY = 0x10000
//};

struct file_operations
{
	int (*llseek)(struct dataserver_file*, size_t offset, int origin);
	int (*read)(struct dataserver_file*, char* buffer, size_t count, size_t offset);
	int (*write)(struct dataserver_file*, char* buffer, size_t count, size_t offset);
	//int (*open)(struct dataserver_file*, int mode); open函数似乎不需要，直接新建一个file_dataserver的对象即可
	//close函数同样不应该在这里出现
};

//only one copy in memory
struct dataserver_super_block
{
	struct super_block* s_block;
	pthread_mutex_t s_mutex;
	struct list_head s_files;
	struct super_block_operations *s_op;
	//You can redefine it
	int hash_arr[12];//haven't finish this function
};

//once open a file, this structure should be built
struct dataserver_file//I think we need a file structure to point opening files
{
	struct list_head* f_open_list;//应该有个双向循环链表
	struct dataserver_super_block *super_block;
	char f_name[256];
	size_t f_cur_offside;
	size_t f_len;
	unsigned int f_mode;
	unsigned int *f_chunks_arr;
	struct file_operations *f_op;
};
#endif
