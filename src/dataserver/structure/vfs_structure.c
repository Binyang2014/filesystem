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
	s_hash_table->hash_table_size = (1<<20) + (1<<19);
	s_hash_table->blocks_arr = (int *)malloc(sizeof(int) * s_hash_table->hash_table_size);
	if(s_hash_table->blocks_arr == 0)
	{
		perror("in init_hash_table function");
		return 1;
	}
	memset(s_hash_table->blocks_arr, -1, sizeof(int) * s_hash_table->hash_table_size);
	return 0;
}

static void init_sb_op(superblock_op_t* s_op)
{
	s_op->get_blocks_count = get_blocks_count;
	s_op->get_free_blocks_count = get_free_blocks_count;
	s_op->get_groups_conut = get_groups_conut;
	s_op->get_filesystem_version = get_filesystem_version;
	s_op->get_last_write_time = get_last_write_time;
	s_op->get_superblock_status = get_superblock_status;

	s_op->find_a_block_num = find_a_block_num;
	s_op->alloc_a_block = alloc_a_block;
	s_op->free_a_block = free_a_block;

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
	dataserver_sb->s_hash_table = (vfs_hashtable_t* )malloc(sizeof(vfs_hashtable_t));
	dataserver_sb->hash_function = PJWHash;//You can alter it as you wish
	dataserver_sb->s_op = (superblock_op_t *)malloc(sizeof(superblock_op_t));
	//initial mutex; maybe is not right, I will review it after this function finish
	if (pthread_mutex_init(&dataserver_sb->s_mutex, NULL) != 0)
	{
		free(dataserver_sb);
		perror("initial dataserver superblock mutex failed");
		return NULL;
	}

	//initial hash table
	if(init_hash_table(dataserver_sb->s_hash_table) != 0)
	{
		free(dataserver_sb);
		return NULL;
	}
	//initial superblock operations
	init_sb_op(dataserver_sb->s_op);

	return dataserver_sb;
}
