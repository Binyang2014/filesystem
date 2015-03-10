/**
 * vfs_operations.c
 * created on: 2015.2.2
 * author: Binyang
 *
 * finish functions needed by operations
 */

#include "vfs_structure.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include "../../tool/hash.h"

//following functions maybe used by read or write function
static void sb_set_block_bm(int block_num);
static void sb_clear_block_bm(int block_num);
//static void sb_regist_block(int chunk_num, int block_num);
//static void sb_logout_block(int chunk_num);

unsigned int get_blocks_count(dataserver_sb_t* this)
{
	return this->s_block->s_blocks_count;
}

unsigned int get_free_blocks_count(dataserver_sb_t* this)
{
	return this->s_block->s_free_blocks_count;
}

float get_filesystem_version(dataserver_sb_t* this)
{
	return this->s_block->s_version;
}

unsigned int get_groups_conut(dataserver_sb_t* this)
{
	return this->s_block->s_groups_count;
}

time_t get_last_write_time(dataserver_sb_t* this)
{
	return this->s_block->s_last_write_time;
}

unsigned short get_superblock_status(dataserver_sb_t* this)
{
	return this->s_block->s_status;
}

//----------------------------------------------------------------------------------
//给定chunk号和block号，通过hash函数查找相应的hash number号
unsigned int find_a_block_num(dataserver_sb_t* this, size_t chunk_num, unsigned int block_num)
{
	char str[MAX_NUM_SIZE + 1];
	unsigned int hash_num;
	ultoa(chunk_num, str);
	hash_num = this->hash_function(str, this->s_hash_table->hash_table_size);

	//Programmer should make sure that at least a value in block_arr is -1
	while(this->s_hash_table->blocks_arr[hash_num] != INF_UNSIGNED_INT &&
			this->s_hash_table->blocks_arr[hash_num] != block_num)
	{
		hash_num = (hash_num + 1) % (this->s_hash_table->hash_table_size);
	}

	if(this->s_hash_table->blocks_arr[hash_num] == block_num)
		return hash_num;

	fprintf(stderr, "Can not find certain chunk_num in this hash table\n");
	return INF_UNSIGNED_INT;
}

//success return 0 fail return other number
int find_serials_blocks(dataserver_sb_t* this, int arr_size,
			size_t* chunks_arr, int* blocks_arr, unsigned int* hash_arr)
{
	int i;
	unsigned int t_hash_number;

	for(i = 0; i < arr_size; i++)
	{
		t_hash_number = find_a_block_num(this, chunks_arr[i], blocks_arr[i]);
		if(t_hash_number != INF_UNSIGNED_INT)
			hash_arr[i] = t_hash_number;
		else
			return 1;
	}
	return 0;
}

//This function just find a suitable place to store a block of data, and return
//the block number
unsigned int alloc_a_block(dataserver_sb_t* this, size_t chunk_num, unsigned int block_num)
{
	unsigned int hash_num;
	char str[MAX_NUM_SIZE + 1];
	ultoa(chunk_num, str);
	hash_num = this->hash_function(str, this->s_hash_table->hash_table_size);

	while(this->s_hash_table->blocks_arr[hash_num] != INF_UNSIGNED_INT)
		hash_num = (hash_num + 1) % this->s_hash_table->hash_table_size;
	if(this->s_hash_table->blocks_arr[hash_num] == INF_UNSIGNED_INT)
	{
		this->s_hash_table->blocks_arr[hash_num] = block_num;
		return hash_num;
	}

	fprintf(stderr, "Can not allocated a block in this file system\n");
	return INF_UNSIGNED_INT;
}

int alloc_blocks(dataserver_sb_t* this, int arr_size,
			size_t* chunks_arr, int* blocks_arr, unsigned int* hash_arr)
{
	int i;
	unsigned int t_hash_num;
	for(i = 0; i < arr_size; i++)
	{
		t_hash_num = alloc_a_block(this, chunks_arr[i], blocks_arr[i]);
		if(t_hash_num != INF_UNSIGNED_INT)
			hash_arr[i] = t_hash_num;
		else
			return 1;
	}
	return 0;
}

//just like above, do not care really blocks and just modifies hash table
unsigned int free_a_block(dataserver_sb_t* this, size_t chunk_num, unsigned int block_num)
{
	unsigned int hash_num;
	hash_num = find_a_block_num(this, chunk_num, block_num);
	if(hash_num == INF_UNSIGNED_INT)
	{
		fprintf(stderr, "free a block fails\n");
		return INF_UNSIGNED_INT;
	}
	this->s_hash_table->blocks_arr[hash_num] = INF_UNSIGNED_INT;
	return hash_num;
}

int free_blocks(dataserver_sb_t* this, int arr_size,
		size_t* chunks_arr, int* blocks_arr, unsigned int* hash_arr)
{
	int i;
	unsigned int t_hash_num;
	for(i = 0; i < arr_size; i++)
	{
		t_hash_num = free_a_block(this, chunks_arr[i], blocks_arr[i]);
		if(t_hash_num != INF_UNSIGNED_INT)
			hash_arr[i] = t_hash_num;
		else
			return 1;
	}
	return 0;
}
//---------------------------------------------------------------------------------

void print_sb_imf(dataserver_sb_t* this)
{
	unsigned int blocks_count = get_blocks_count(this);
	unsigned int free_blocks_count = get_free_blocks_count(this);
	float filesystem_version = get_filesystem_version(this);
	unsigned int groups_conut = get_groups_conut(this);
	time_t last_write_time = get_last_write_time(this);
	unsigned short status = get_superblock_status(this);
	struct tm *timeinfo;
	timeinfo = localtime(&last_write_time);

	printf("The number of blocks in this file system is %u\n", blocks_count);
	printf("The number of free blocks in this file system is %u\n", free_blocks_count);
	printf("The version of this file system is %.1f\n", filesystem_version);
	printf("The number of groups in this file system is %u\n", groups_conut);
	printf("The last write time to this file system is %s", asctime(timeinfo));
	printf("The status of this file system is %u\n", status);
}
