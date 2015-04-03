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
#define VFS_HASH_TBALE_SIZE ((1<<20) + (1<<19))//define the size of hash table

//simple function used by read
#define ALL_ADD_THIRD(c, r, t)   \
(                                  \
    (c) = (c) + (t),               \
    (r) = (r) + (t)                \
)
#define VFS_READ 0x01
#define VFS_WRITE 0x10


#include <pthread.h>
#include <sys/types.h>
#include "basic_structure.h"

struct dataserver_super_block;
struct dataserver_file;
struct dataserver_block;

//struct list_head
//{
//	struct list_head *pre;
//	struct list_head *next;
//};

struct vfs_hashtable
{
	int hash_table_size;
	unsigned int *blocks_arr;
	unsigned long long *chunks_arr;
};

struct super_block_operations
{
	unsigned int (*get_blocks_count)(struct dataserver_super_block*);
	unsigned int (*get_free_blocks_count)(struct dataserver_super_block*);
	unsigned int (*get_blocks_per_groups)(struct dataserver_super_block*);
	float (*get_filesystem_version)(struct dataserver_super_block*);
	unsigned int (*get_groups_conut)(struct dataserver_super_block*);
	time_t (*get_last_write_time)(struct dataserver_super_block*);
	unsigned short (*get_superblock_status)(struct dataserver_super_block*);
	unsigned int (*get_per_group_reserved)(struct dataserver_super_block*);

	unsigned int(*find_a_block_num)(struct dataserver_super_block*, unsigned long long chunk_num);
	unsigned int* (*find_serials_blocks)(struct dataserver_super_block*, int arr_size,
			unsigned long long* chunks_arr, unsigned int* blocks_arr);

	//use chunks number and blocks number to contribute hash table, so it will be convenient to search
	//these functions do use really write to blocks or read from blocks
	int (*alloc_blocks)(struct dataserver_super_block*, int arr_size,
				unsigned long long* chunks_arr, unsigned int* blocks_arr);
	unsigned int* (*alloc_blocks_with_hash)(struct dataserver_super_block*, int arr_size,
			unsigned long long* chunks_arr, unsigned int* blocks_arr, unsigned int* hash_arr);
	int (*free_blocks)(struct dataserver_super_block*, int arr_size, unsigned long long* chunks_arr);
	unsigned int* (*free_blocks_with_return)(struct dataserver_super_block*, int arr_size,
			unsigned long long* chunks_arr, unsigned int* blocks_arr);
	//success return hash number else return -1
	unsigned int (*alloc_a_block)(struct dataserver_super_block*, unsigned long long chunk_num, unsigned int block_num);
	unsigned int (*free_a_block)(struct dataserver_super_block*, unsigned long long chunk_num);

	void (*print_sb_imf)(struct dataserver_super_block*);
	//set chunk and block
	//clear chunk and block
};

/**
 * data server do not save the status of an opening file only offers basic read and write operations
 * a opening file's status should be managed by client
 *
 * only one  copy, there are functions!!!
 */
struct file_operations
{
	int (*vfs_read)(struct dataserver_file*, char* buffer, size_t count, off_t offset);
	int (*vfs_write)(struct dataserver_file*, char* buffer, size_t count, off_t offset);
	//delete function has not been well defined yet, I will redefine it after test read and write
	int (*vfs_delete)(struct dataserver_file*);
};

//only one copy in memory
struct dataserver_super_block
{
	super_block_t* s_block;
	pthread_mutex_t s_mutex;

	struct super_block_operations *s_op;

	struct vfs_hashtable *s_hash_table;
	unsigned int (*hash_function)(char* str, unsigned int size);
};

/**
 * reconsider this structure. In data server, we do not care if the args is right,
 * before send command to data server, the args should be checked and make sure
 * there is no error. The length of file is no more needed
*/
struct dataserver_file//I think we need a file structure to point opening files
{
	struct dataserver_super_block *super_block;
	//off_t f_cur_offset;
	int arr_len;
	unsigned long long *f_chunks_arr;
	unsigned int *f_blocks_arr;
	struct file_operations *f_op;
};

typedef struct dataserver_super_block dataserver_sb_t;
typedef struct dataserver_file dataserver_file_t;
typedef struct vfs_hashtable vfs_hashtable_t;
typedef struct super_block_operations superblock_op_t;
typedef struct file_operations file_op_t;

//these function I want to be used inline
inline static unsigned long* __get_bitmap_block(super_block_t* super_block, unsigned int block_num)
{
	unsigned int group_id = block_num / super_block->s_blocks_per_group;
	unsigned int bitmap_block_num;
	unsigned long *bitmap;
	group_desc_block_t *group_desc;

	group_desc = (group_desc_block_t *)((char*)super_block + BLOCK_SIZE);
	group_desc = group_desc + group_id;
	bitmap_block_num = group_desc->bg_block_bitmap;
	bitmap = (unsigned long *)((char *)super_block + (BLOCK_SIZE * bitmap_block_num));
	return bitmap;
}
inline static unsigned long* __get_bitmap_from_gid(super_block_t* super_block, unsigned int group_id)
{
	unsigned int bitmap_block_num;
	unsigned long *bitmap;
	group_desc_block_t *group_desc;

	group_desc = (group_desc_block_t *)((char*)super_block + BLOCK_SIZE);
	group_desc = group_desc + group_id;
	bitmap_block_num = group_desc->bg_block_bitmap;
	bitmap = (unsigned long *)((char *)super_block + (BLOCK_SIZE * bitmap_block_num));
	return bitmap;
}

//following functions is implemented in file vfs_operations.c about super block
unsigned int get_blocks_count(dataserver_sb_t* this);
unsigned int get_free_blocks_count(dataserver_sb_t* this);
unsigned int get_blocks_per_groups(dataserver_sb_t* this);
float get_filesystem_version(dataserver_sb_t* this);
unsigned int get_groups_conut(dataserver_sb_t* this);
time_t get_last_write_time(dataserver_sb_t* this);
unsigned short get_superblock_status(dataserver_sb_t* this);
unsigned int get_per_group_reserved(dataserver_sb_t* this);

unsigned int find_a_block_num(dataserver_sb_t* this, unsigned long long chunk_num);
unsigned int* find_serials_blocks(struct dataserver_super_block*, int arr_size,
			unsigned long long* chunks_arr, unsigned int* blocks_arr);
unsigned int alloc_a_block(dataserver_sb_t* this, unsigned long long chunk_num, unsigned int block_num);
int alloc_blocks(dataserver_sb_t* this, int arr_size,
			unsigned long long* chunks_arr, unsigned int* blocks_arr);
unsigned int* alloc_blocks_with_hash(dataserver_sb_t* this, int arr_size,
			unsigned long long* chunks_arr, unsigned int* blocks_arr, unsigned int* hash_arr);
unsigned int free_a_block(dataserver_sb_t* this, unsigned long long chunk_num);
unsigned int* free_blocks_with_return(dataserver_sb_t* this, int arr_size,
		unsigned long long* chunks_arr, unsigned int* blocks_arr);
int free_blocks(dataserver_sb_t* this, int arr_size, unsigned long long* chunks_arr);

void print_sb_imf(dataserver_sb_t* this);

//following functions is implemented in file vfs_operations.c about file operations
int vfs_read(dataserver_file_t*, char* buffer, size_t count, off_t offset);
int vfs_write(dataserver_file_t*, char* buffer, size_t count, off_t offset);

//following functions will be finished in file vfs_structure.c
//this function should be called first
dataserver_sb_t* vfs_init(total_size_t t_size, int dev_num);
//buffer provide by data server, it can not be null
dataserver_file_t* init_vfs_file(dataserver_sb_t*, dataserver_file_t*,
		vfs_hashtable_t* arr_table, short mode);
#endif
