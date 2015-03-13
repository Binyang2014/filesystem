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

//file open mode, there are no open function here
#define VFS_READ = 0x01
#define VFS_WRITE = 0x10
#define VFS_APPEND = 0x100
#define VFS_TEXT = 0x1000
#define VFS_BINARY = 0x10000
//simple function used by read
#define ALL_ADD_THIRD(c, r, t)   \
(                                  \
    (c) = (c) + (t),               \
    (r) = (r) + (t)                \
)

#include <pthread.h>
#include "basic_structure.h"

struct dataserver_super_block;
struct dataserver_file;
struct dataserver_block;

enum seek_pos
{
	VFS_SEEK_SET,
	VFS_SEEK_CUR,
	VFS_SEEK_END
};

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
	unsigned int (*get_blocks_per_groups)(struct dataserver_super_block*);
	float (*get_filesystem_version)(struct dataserver_super_block*);
	unsigned int (*get_groups_conut)(struct dataserver_super_block*);
	time_t (*get_last_write_time)(struct dataserver_super_block*);
	unsigned short (*get_superblock_status)(struct dataserver_super_block*);
	unsigned int (*get_per_group_reserved)(struct dataserver_super_block*);

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


struct file_operations
{
	size_t (*vfs_llseek)(struct dataserver_file*, off_t offset, seek_pos_t origin);
	int (*vfs_read)(struct dataserver_file*, char* buffer, size_t count, off_t offset);
	int (*vfs_write)(struct dataserver_file*, char* buffer, size_t count, off_t offset);
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
//process use fd to control file not file name
struct dataserver_file//I think we need a file structure to point opening files
{
	struct list_head* f_open_list;//Should be a two-way circular list
	struct dataserver_super_block *super_block;
	off_t f_cur_offset;
	size_t f_len;
	unsigned int f_mode;
	unsigned int *f_chunks_arr;
	unsigned int *f_blocks_arr;
	struct file_operations *f_op;
};

typedef struct dataserver_super_block dataserver_sb_t;
typedef struct dataserver_file dataserver_file_t;
typedef struct vfs_hashtable vfs_hashtable_t;
typedef struct super_block_operations superblock_op_t;
typedef struct list_head list_head_t;
typedef enum seek_pos seek_pos_t;

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

//following functions is implemented in file vfs_operations.c about file operations
int vfs_read(dataserver_file_t*, char* buffer, size_t count, off_t offset);
int vfs_write(dataserver_file_t*, char* buffer, size_t count, off_t offset);
off_t vfs_llseek(dataserver_file_t*,, off_t offset, seek_pos_t origin);

//following functions will be finished in file vfs_structure.c
dataserver_sb_t * init_vfs_sb(char* filesystem);
#endif
