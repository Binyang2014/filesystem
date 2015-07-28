/*
 * vfs_structure.c
 *
 * created on: 2015.3.8
 * author: Binyang
 *
 * this file implement functions in vfs_structure.h
 */
#include "vfs_structure.h"
#include "../../common/structure_tool/map.h"
#include "../../common/structure_tool/log.h"
#include "../../common/structure_tool/zmalloc.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static file_op_t* file_op;
static char* filesystem;

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
	s_op->free_a_block = free_a_block;
	s_op->alloc_blocks = alloc_blocks;
	s_op->free_blocks_with_return = free_blocks_with_return;
	s_op->free_blocks = free_blocks;

	s_op->print_sb_imf = print_sb_imf;
}

//init file operations
static void init_file_op(file_op_t* f_op)
{
	f_op->vfs_read = vfs_read;
	f_op->vfs_write = vfs_write;
	f_op->vfs_remove = vfs_remove;
}

//follows are some functions that will be used by hash table
static void* hash_table_pair_dup(void* pair)
{
	pair_t *p = zmalloc(sizeof(pair_t));
	uint32_t* value_t = NULL;

	p->key = sds_dup(((pair_t *)pair)->key);
	value_t = zmalloc(sizeof(uint32_t));
	*value_t = *(uint32_t *)(((pair_t *)pair)->value);
	p->value = value_t;
	return (void*)p;
}

static void hash_table_pair_free(void* pair)
{
	pair_t *p = pair;
	sds_free(p->key);
	zfree(p->value);
	zfree(p);
}

static void* hash_table_value_dup(const void* value)
{
	uint32_t *value_t = zmalloc(sizeof(uint32_t));
	*value_t = *((uint32_t*)value);
	return (void*)value_t;
}

static void hash_table_value_free(void* value)
{
	uint32_t* value_t = value;
	zfree(value_t);
}

static dataserver_sb_t * init_vfs_sb()
{
	dataserver_sb_t *dataserver_sb;
	dataserver_sb = (dataserver_sb_t *)zmalloc(sizeof(dataserver_sb_t));
	if(dataserver_sb == NULL)
	{
		perror("malloc dataserver superblock failed");
		return NULL;
	}

	dataserver_sb->s_block = (super_block_t *)filesystem;
	dataserver_sb->s_hash_table = NULL;
	dataserver_sb->s_op = NULL;

	//initial mutex; maybe is not right, I will review it after this function finish
	if (pthread_mutex_init(&dataserver_sb->s_mutex, NULL) != 0)
	{
		zfree(dataserver_sb);
		perror("initial dataserver superblock mutex failed");
		return NULL;
	}

	//initial hash table
	dataserver_sb->s_hash_table = create_map(VFS_HASH_TABLE_SIZE, hash_table_value_dup, 
			hash_table_value_free, hash_table_pair_dup, hash_table_pair_free);
	if(dataserver_sb->s_hash_table == NULL)
	{
		zfree(dataserver_sb);
		return NULL;
	}


	//initial superblock operations
	dataserver_sb->s_op = (superblock_op_t *)zmalloc(sizeof(superblock_op_t));
	if(dataserver_sb->s_op == NULL)
	{
		destroy_map(dataserver_sb->s_hash_table);
		zfree(dataserver_sb);
		perror("allocate super block operations failed");
		return NULL;
	}
	init_sb_op(dataserver_sb->s_op);

	return dataserver_sb;
}

//set value to some static variable
dataserver_sb_t* vfs_init(total_size_t t_size, int dev_num)
{

	dataserver_sb_t *dataserver_sb;

	filesystem = NULL;
	file_op = NULL;

	file_op = (file_op_t *)zmalloc(sizeof(file_op_t));
	if(file_op == NULL)
	{
		err_ret("in vfs_basic_init");
		return NULL;
	}
	init_file_op(file_op);

	filesystem = init_mem_file_system(t_size, dev_num);
	dataserver_sb = init_vfs_sb();
	if(filesystem == NULL || dataserver_sb == NULL)
		return NULL;

	return dataserver_sb;
}

void vfs_destroy(dataserver_sb_t * dataserver_sb)
{
	zfree(dataserver_sb->s_op);
	destroy_map(dataserver_sb->s_hash_table);
	zfree(dataserver_sb);
	free_mem_file_system(filesystem);
	zfree(file_op);
}

//=======================some operations about vfs file============
dataserver_file_t* init_vfs_file(dataserver_sb_t* super_block, dataserver_file_t* v_file_buff,
		 vfs_hashtable_t* arr_table, short mode)
{
	if(v_file_buff == NULL)
	{
		err_msg("args wrong in init_vfs_file");
		return NULL;
	}
	v_file_buff->super_block = super_block;
	//v_file_buff->f_cur_offset = offset;
	v_file_buff->arr_len = arr_table->hash_table_size;
	v_file_buff->f_blocks_arr = arr_table->blocks_arr;
	v_file_buff->f_chunks_arr = arr_table->chunks_arr;
	v_file_buff->f_op = file_op;

	if(mode == VFS_READ)
	{
		if( v_file_buff->super_block->s_op->find_serials_blocks(v_file_buff->super_block, v_file_buff->arr_len,
				v_file_buff->f_chunks_arr, v_file_buff->f_blocks_arr)==NULL )
		{
			err_msg("in init_vfs_file, something wrong");
			return NULL;
		}
		else
			return v_file_buff;
	}
	return v_file_buff;
}

//==================some operastions about vfs hash table==========
vfs_hashtable_t* init_hashtable(int table_size)
{
	vfs_hashtable_t* hash_table = (vfs_hashtable_t* )zmalloc(sizeof(vfs_hashtable_t));
	hash_table->hash_table_size = table_size;
	hash_table->chunks_arr = (uint64_t* )zmalloc(sizeof(uint64_t) * table_size);
	hash_table->blocks_arr = (uint32_t* )zmalloc(sizeof(uint32_t) * table_size);
	return hash_table;
}

void destroy_hashtable(vfs_hashtable_t* hash_table)
{
	zfree(hash_table->chunks_arr);
	zfree(hash_table->blocks_arr);
	zfree(hash_table);
}
