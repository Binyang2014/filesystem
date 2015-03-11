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
#define VFS_HASH_TBALE_SIZE ((1<<20) + (1<<19))

#include <pthread.h>
#include "basic_structure.h"

struct dataserver_super_block;
struct dataserver_file;
struct dataserver_block;

struct list_head
{
	struct list_head *pre;
	struct list_head *next;
};

struct vfs_hashtable
{
	int hash_table_size;
	unsigned int *blocks_arr;
	size_t *chunks_arr;
};

struct super_block_operations
{
	unsigned int (*get_blocks_count)(struct dataserver_super_block*);
	unsigned int (*get_free_blocks_count)(struct dataserver_super_block*);
	float (*get_filesystem_version)(struct dataserver_super_block*);
	unsigned int (*get_groups_conut)(struct dataserver_super_block*);
	time_t (*get_last_write_time)(struct dataserver_super_block*);
	unsigned short (*get_superblock_status)(struct dataserver_super_block*);

	unsigned int(*find_a_block_num)(struct dataserver_super_block*, size_t chunk_num);
	unsigned int* (*find_serials_blocks)(struct dataserver_super_block*, int arr_size,
			size_t* chunks_arr, unsigned int* blocks_arr);

	//use chunks number and blocks number to contribute hash table, so it will be convenient to search
	//these functions do use really write to blocks or read from blocks
	int (*alloc_blocks)(struct dataserver_super_block*, int arr_size,
				size_t* chunks_arr, unsigned int* blocks_arr);
	unsigned int* (*alloc_blocks_with_hash)(struct dataserver_super_block*, int arr_size,
			size_t* chunks_arr, unsigned int* blocks_arr, unsigned int* hash_arr);
	int (*free_blocks)(struct dataserver_super_block*, int arr_size, size_t* chunks_arr);
	unsigned int* (*free_blocks_with_return)(struct dataserver_super_block*, int arr_size, size_t* chunks_arr,
			unsigned int* blocks_arr);
	//success return block number else return -1
	unsigned int (*alloc_a_block)(struct dataserver_super_block*, size_t chunk_num, unsigned int block_num);;
	unsigned int (*free_a_block)(struct dataserver_super_block*, size_t chunk_num);

	void (*print_sb_imf)(struct dataserver_super_block*);
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

//used to cache temporary data
struct dataserver_block
{
	char block[BLOCK_SIZE];
};

//only one copy in memory
struct dataserver_super_block
{
	super_block_t* s_block;
	pthread_mutex_t s_mutex;
	struct list_head *s_files;
	struct super_block_operations *s_op;

	struct vfs_hashtable *s_hash_table;
	unsigned int (*hash_function)(char* str, unsigned int size);
};

//once open a file, this structure should be built
struct dataserver_file//I think we need a file structure to point opening files
{
	struct list_head* f_open_list;//Should be a two-way circular list
	struct dataserver_super_block *super_block;
	char f_name[256];
	size_t f_cur_offside;
	size_t f_len;
	unsigned int f_mode;
	unsigned int *f_chunks_arr;
	struct file_operations *f_op;
};

typedef struct dataserver_super_block dataserver_sb_t;
typedef struct dataserver_file dataserver_file_t;
typedef struct vfs_hashtable vfs_hashtable_t;
typedef struct super_block_operations superblock_op_t;
typedef struct list_head list_head_t;

//following functions is implement in file vfs_operations.c about super block
unsigned int get_blocks_count(dataserver_sb_t* this);
unsigned int get_free_blocks_count(dataserver_sb_t* this);
float get_filesystem_version(dataserver_sb_t* this);
unsigned int get_groups_conut(dataserver_sb_t* this);
time_t get_last_write_time(dataserver_sb_t* this);
unsigned short get_superblock_status(dataserver_sb_t* this);

unsigned int find_a_block_num(dataserver_sb_t* this, size_t chunk_num);
unsigned int* find_serials_blocks(struct dataserver_super_block*, int arr_size,
			size_t* chunks_arr, unsigned int* blocks_arr);
unsigned int alloc_a_block(dataserver_sb_t* this, size_t chunk_num, unsigned int block_num);
int alloc_blocks(dataserver_sb_t* this, int arr_size,
			size_t* chunks_arr, unsigned int* blocks_arr);
unsigned int* alloc_blocks_with_hash(dataserver_sb_t* this, int arr_size,
			size_t* chunks_arr, unsigned int* blocks_arr, unsigned int* hash_arr);
unsigned int free_a_block(dataserver_sb_t* this, size_t chunk_num);
unsigned int* free_blocks_with_return(dataserver_sb_t* this, int arr_size,
		size_t* chunks_arr, unsigned int* blocks_arr);
int free_blocks(dataserver_sb_t* this, int arr_size, size_t* chunks_arr);

void print_sb_imf(dataserver_sb_t* this);

//following functions will be finished in file vfs_structure.c
dataserver_sb_t * init_vfs_sb(char* filesystem);
#endif
