/*
 * vfs_structure.c
 *
 * created on: 2015.3.8
 * author: Binyang
 *
 * this file implement functions in vfs_structure.h
 */
#include "vfs_structure.h"
#include "../../tool/hash.h"
#include <stdio.h>
#include <string.h>

static int init_hash_table(vfs_hashtable_t* s_hash_table)
{
	//Max file system size is 2^32 and minimum block size is 2^12
	//we need at least 2^20 to store array, also we need to leave
	//enough room to store INF_UNSIGNED_INT to make hash table as fast as possible
	s_hash_table->hash_table_size = VFS_HASH_TBALE_SIZE;
	s_hash_table->blocks_arr = (unsigned int* )malloc(sizeof(unsigned int) * s_hash_table->hash_table_size);
	s_hash_table->chunks_arr = (size_t* )malloc(sizeof(size_t) * s_hash_table->hash_table_size);
	if(s_hash_table->blocks_arr == NULL || s_hash_table->chunks_arr == NULL)
	{
		perror("in init_hash_table function");
		return 1;
	}
	memset(s_hash_table->blocks_arr, -1, sizeof(unsigned int) * s_hash_table->hash_table_size);
	memset(s_hash_table->chunks_arr, -1, sizeof(size_t) * s_hash_table->hash_table_size);
	return 0;
}

static list_head_t* alloc_list_head()
{
	list_head_t* list_head;
	list_head = (list_head_t* )malloc(sizeof(list_head_t));
	if(list_head == NULL)
	{
		perror("in init_list_head");
		return NULL;
	}
	list_head->next = list_head;
	list_head->pre = list_head;
	return list_head;
}

static void init_sb_op(superblock_op_t* s_op)
{
	s_op->get_blocks_count = get_blocks_count;
	s_op->get_free_blocks_count = get_free_blocks_count;
	s_op->get_groups_conut = get_groups_conut;
	s_op->get_blocks_per_groups = get_blocks_per_groups;
	s_op->get_filesystem_version = get_filesystem_version;
	s_op->get_last_write_time = get_last_write_time;
	s_op->get_superblock_status = get_superblock_status;
	s_op->get_per_group_reserved = get_per_group_reserved;

	s_op->find_a_block_num = find_a_block_num;
	s_op->find_serials_blocks = find_serials_blocks;
	s_op->alloc_a_block = alloc_a_block;
	s_op->alloc_blocks_with_hash = alloc_blocks_with_hash;
	s_op->free_a_block = free_a_block;
	s_op->alloc_blocks = alloc_blocks;
	s_op->free_blocks_with_return = free_blocks_with_return;
	s_op->free_blocks = free_blocks;

	s_op->print_sb_imf = print_sb_imf;
}

dataserver_sb_t * init_vfs_sb(char* filesystem)
{
	dataserver_sb_t *dataserver_sb;
	dataserver_sb = (dataserver_sb_t *)malloc(sizeof(dataserver_sb_t));
	if(dataserver_sb == NULL)
	{
		perror("malloc dataserver superblock failed");
		return NULL;
	}

	dataserver_sb->s_files = NULL;
	dataserver_sb->s_block = (super_block_t *)filesystem;
	dataserver_sb->hash_function = PJWHash;//You can alter it as you wish
	dataserver_sb->s_hash_table = NULL;
	dataserver_sb->s_op = NULL;

	//initial mutex; maybe is not right, I will review it after this function finish
	if (pthread_mutex_init(&dataserver_sb->s_mutex, NULL) != 0)
	{
		free(dataserver_sb);
		perror("initial dataserver superblock mutex failed");
		return NULL;
	}

	//initial hash table
	dataserver_sb->s_hash_table = (vfs_hashtable_t* )malloc(sizeof(vfs_hashtable_t));
	if(dataserver_sb->s_hash_table == NULL)
	{
		free(dataserver_sb);
		perror("allocate hash_table failed");
		return NULL;
	}
	if(init_hash_table(dataserver_sb->s_hash_table) != 0)
	{
		free(dataserver_sb);
		return NULL;
	}

	//initial head list
	if((dataserver_sb->s_files = alloc_list_head()) == NULL)
	{
		free(dataserver_sb);
		return NULL;
	}

	//initial superblock operations
	dataserver_sb->s_op = (superblock_op_t *)malloc(sizeof(superblock_op_t));
	if(dataserver_sb->s_op == NULL)
	{
		free(dataserver_sb);
		perror("allocate super block operations failed");
		return NULL;
	}
	init_sb_op(dataserver_sb->s_op);

	return dataserver_sb;
}

