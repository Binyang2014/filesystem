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
#include <string.h>
#include <pthread.h>
#include <stdint.h>
#include "sds.h"
#include "bitmap.h"
#include "map.h"
#include "log.h"
#include "zmalloc.h"

//------------------------basic function--------------------
static char* find_a_block(dataserver_sb_t* dataserver_sb, uint32_t block_num)
{
	uint32_t blocks_per_groups;
	int group_offset;
	super_block_t *super_block = dataserver_sb->s_block;
	char* block;
	blocks_per_groups = dataserver_sb->s_op->get_blocks_per_groups(dataserver_sb);
	group_offset = block_num % blocks_per_groups;
	if(group_offset  < dataserver_sb->s_op->get_per_group_reserved(dataserver_sb))
	{
		fprintf(stderr, "can not read reserved information\n");
		return NULL;
	}
	//may be we should check if this block's bitmap is set, we are not trust server
	block = (char* )super_block + block_num * BLOCK_SIZE;
	return block;
}

//----------------------super block operations----------------------------------------
uint32_t get_blocks_count(dataserver_sb_t* this)
{
	return this->s_block->s_blocks_count;
}

uint32_t get_free_blocks_count(dataserver_sb_t* this)
{
	return this->s_block->s_free_blocks_count;
}

uint32_t get_blocks_per_groups(dataserver_sb_t* this)
{
	return this->s_block->s_blocks_per_group;
}

float get_filesystem_version(dataserver_sb_t* this)
{
	return this->s_block->s_version;
}

uint32_t get_groups_conut(dataserver_sb_t* this)
{
	return this->s_block->s_groups_count;
}

time_t get_last_write_time(dataserver_sb_t* this)
{
	return this->s_block->s_last_write_time;
}

uint16_t get_superblock_status(dataserver_sb_t* this)
{
	return this->s_block->s_status;
}

uint32_t get_per_group_reserved(dataserver_sb_t* this)
{
	return this->s_block->s_per_group_reserved;
}

//----------------------------------------------------------------------------------
//给定chunk号，通过hash函数查找相应的block number号
uint32_t find_a_block_num(dataserver_sb_t* this, uint64_t chunk_num)
{
	uint32_t* block_num_p;
	sds key = sds_new_ull(chunk_num);
	block_num_p = this->s_hash_table->op->get(this->s_hash_table, key);
	sds_free(key);

	if(block_num_p != NULL)
	{
		uint32_t block_num = *block_num_p;
		zfree(block_num_p);
		return block_num;
	}
	return INF_UNSIGNED_INT;
}

//success return address of array fail return null
uint32_t* find_serials_blocks(dataserver_sb_t* this, int arr_size,
			uint64_t* chunks_arr, uint32_t* blocks_arr)
{
	int i;
	uint32_t t_block_number;

	for(i = 0; i < arr_size; i++)
	{
		t_block_number = find_a_block_num(this, chunks_arr[i]);
		if(t_block_number != INF_UNSIGNED_INT)
			blocks_arr[i] = t_block_number;
		else
			return NULL;
	}
	return blocks_arr;
}

//This function just find a suitable place to store a block of data, and return
//0 if success -1 if fail
int alloc_a_block(dataserver_sb_t* this, uint64_t chunk_num, uint32_t block_num)
{
	uint32_t* block_num_p;
	sds key = sds_new_ull(chunk_num);

	block_num_p = this->s_hash_table->op->get(this->s_hash_table, key);

	if(block_num_p == NULL)
	{
		this->s_hash_table->op->put(this->s_hash_table, key, &block_num);
		sds_free(key);
		return 0;
	}

	fprintf(stderr, "Can not allocated a block in this file system\n");
	return -1;
}

int alloc_blocks(dataserver_sb_t* this, int arr_size,
			uint64_t* chunks_arr, uint32_t* blocks_arr)
{
	int i;
	uint32_t t_hash_num;
	for(i = 0; i < arr_size; i++)
	{
		t_hash_num = alloc_a_block(this, chunks_arr[i], blocks_arr[i]);
		if(t_hash_num != INF_UNSIGNED_INT)
			continue;
		else
			return INF_UNSIGNED_INT;
	}
	return 0;
}

//just like above, do not care really blocks and just modifies hash table
uint32_t free_a_block(dataserver_sb_t* this, uint64_t chunk_num)
{
	sds key = sds_new_ull(chunk_num);
	uint32_t* block_num_p;
	block_num_p = this->s_hash_table->op->get(this->s_hash_table, key);
	this->s_hash_table->op->del(this->s_hash_table, key);
	sds_free(key);
	if(block_num_p != NULL)
	{
		uint32_t block_num = *block_num_p;
		zfree(block_num_p);
		return block_num;
	}
	return INF_UNSIGNED_INT;
}

uint32_t* free_blocks_with_return(dataserver_sb_t* this, int arr_size,
		uint64_t* chunks_arr, uint32_t* blocks_arr)
{
	int i;
	uint32_t t_block_num;
	for(i = 0; i < arr_size; i++)
	{
		t_block_num = free_a_block(this, chunks_arr[i]);
		if(t_block_num != INF_UNSIGNED_INT)
			blocks_arr[i] = t_block_num;
		else
			return NULL;
	}
	return blocks_arr;
}

//free blocks, if success return 0 else return -1
int free_blocks(dataserver_sb_t* this, int arr_size, uint64_t* chunks_arr)
{
	int i;
	uint32_t t_block_num;
	for(i = 0; i < arr_size; i++)
	{
		t_block_num = free_a_block(this, chunks_arr[i]);
		if(t_block_num != INF_UNSIGNED_INT)
			continue;
		else
			return -1;
	}
	return 0;
}
//---------------------------------------------------------------------------------

void print_sb_imf(dataserver_sb_t* this)
{
	uint32_t blocks_count = get_blocks_count(this);
	uint32_t free_blocks_count = get_free_blocks_count(this);
	uint32_t blocks_per_group = get_blocks_per_groups(this);
	float filesystem_version = get_filesystem_version(this);
	uint32_t groups_conut = get_groups_conut(this);
	time_t last_write_time = get_last_write_time(this);
	uint16_t status = get_superblock_status(this);
	struct tm *timeinfo;
	timeinfo = localtime(&last_write_time);

	printf("The number of blocks in this file system is %u\n", blocks_count);
	printf("The number of free blocks in this file system is %u\n", free_blocks_count);
	printf("The number of blocks per groups is %u\n", blocks_per_group);
	printf("The version of this file system is %.1f\n", filesystem_version);
	printf("The number of groups in this file system is %u\n", groups_conut);
	printf("The last write time to this file system is %s", asctime(timeinfo));
	printf("The status of this file system is %u\n", status);
}

//-------------------------file operations------------------------------------------------------

static int cal_first_bytes(off_t offset)
{
	int first_nbytes = BLOCK_SIZE - (offset % BLOCK_SIZE);
	if(first_nbytes == BLOCK_SIZE)
		first_nbytes = 0;
	return first_nbytes;
}


static int read_n_bytes(dataserver_file_t *this, char* buffer, int nbytes, off_t offset)
{
	uint32_t *blocks_arr;
	uint32_t block_num;
	off_t offset_in_group;

#if VFS_RW_DEBUG
	char* alloced_block;
	uint64_t chunk_num;
#endif

	blocks_arr = this->f_blocks_arr;
	block_num = blocks_arr[offset / BLOCK_SIZE];
	offset_in_group = offset % BLOCK_SIZE;

	memcpy(buffer, find_a_block(this->super_block, block_num) + offset_in_group, nbytes);

#if VFS_RW_DEBUG
	printf("--------------VFS_RW_DEBUG READ PART------------------\n");
	alloced_block = find_a_block(this->super_block, block_num);
	printf("The address of alloced block is %p\n", alloced_block);
	chunk_num = this->f_chunks_arr[offset / BLOCK_SIZE];
	printf("The block number is %d, and the chunk number is %llu\n", block_num, (unsigned long long)chunk_num);
	printf("the buffer is %s\n", buffer);
#endif

	return nbytes;
}

int vfs_read(dataserver_file_t *this, char* buffer, size_t count, off_t offset)
{
	int nblocks, last_nbytes, first_nbytes;
	int nbytes_read = 0, nbytes_temp = 0;
	off_t cur_offset;
	int i;

	cur_offset = offset;//set current offset to offset

	first_nbytes = cal_first_bytes(offset);

	//allow read nothing
	if(count == 0)
		return 0;

	//if need data only in one block
	if(count <= first_nbytes)
	{
		first_nbytes = count;
		if((nbytes_temp = read_n_bytes(this, buffer + nbytes_read, first_nbytes, cur_offset)) == -1)
			return -1;
		ALL_ADD_THIRD(cur_offset, nbytes_read, nbytes_temp);
		//this->f_cur_offset = cur_offset;
		return nbytes_read;
	}

	//read first bytes if first number of bytes does not equal to 0
	if( first_nbytes && (nbytes_temp = read_n_bytes(this, buffer + nbytes_read, first_nbytes,
			cur_offset)) == -1)
		return -1;
	ALL_ADD_THIRD(cur_offset, nbytes_read, nbytes_temp);

	//read blocks of data
	nblocks = (count - first_nbytes) / BLOCK_SIZE;
	last_nbytes = (count - first_nbytes) % BLOCK_SIZE;

	for (i = 0; i < nblocks; i++)
	{
		if((nbytes_temp = read_n_bytes(this, buffer + nbytes_read, BLOCK_SIZE,
				cur_offset)) == -1)
			return -1;
		ALL_ADD_THIRD(cur_offset, nbytes_read, nbytes_temp);
	}

	//read rest of data
	nbytes_temp = 0;
	if( last_nbytes && (nbytes_temp = read_n_bytes(this, buffer + nbytes_read,
			last_nbytes, cur_offset)) == -1)
		return -1;
	ALL_ADD_THIRD(cur_offset, nbytes_read, nbytes_temp);
	//this->f_cur_offset = cur_offset;
	return nbytes_read;
}

static int write_n_bytes(dataserver_file_t *this, char* buffer, int nbytes, off_t offset)
{
	uint32_t* blocks_arr;
	uint32_t block_num;
	off_t offset_in_group;

#if VFS_RW_DEBUG
	char* alloced_block;
#endif

	blocks_arr = this->f_blocks_arr;
	block_num = blocks_arr[offset / BLOCK_SIZE];
	offset_in_group = offset % BLOCK_SIZE;

	//memcpy do not change super block, so we can put it out
	memcpy(find_a_block(this->super_block, block_num) + offset_in_group, buffer, nbytes);

#if VFS_RW_DEBUG
	uint64_t chunk_num;
	printf("--------------VFS_RW_DEBUG WRITE PART-----------------\n");
	alloced_block = find_a_block(this->super_block, block_num);
	printf("The address of alloced block is %p\n", alloced_block);
	chunk_num = this->f_chunks_arr[offset / BLOCK_SIZE];
	printf("The block number is %d, and the chunk number is %llu\n", block_num, (unsigned long long)chunk_num);
	printf("the buffer is %s\n", buffer);
	//printf("The block contains %s\n", alloced_block + offset_in_group);
#endif
	return nbytes;
}


int vfs_write(dataserver_file_t* this, char* buffer, size_t count, off_t offset)
{
	int nblocks, last_nbytes, first_nbytes;
	int nbytes_write = 0, nbytes_temp = 0;
	off_t cur_offset;
	int i;

	cur_offset = offset;

	first_nbytes = cal_first_bytes(offset);

	//allow write nothing
	if(count == 0)
		return 0;

	//only need to write data to one block
	if(count <= first_nbytes)
	{
		first_nbytes = count;
		if((nbytes_temp = write_n_bytes(this, buffer + nbytes_write, first_nbytes, cur_offset)) == -1)
				return -1;
		ALL_ADD_THIRD(cur_offset, nbytes_write, nbytes_temp);
		//this->f_cur_offset = cur_offset;
		return nbytes_write;
	}

	//write first bytes
	if(first_nbytes && (nbytes_temp = write_n_bytes(this, buffer + nbytes_write,
			first_nbytes, cur_offset)) == -1)
		return -1;
	ALL_ADD_THIRD(cur_offset, nbytes_write, nbytes_temp);

	//write blocks of data
	nblocks = (count - first_nbytes) / BLOCK_SIZE;
	last_nbytes = (count - first_nbytes) % BLOCK_SIZE;

	for (i = 0; i < nblocks; i++)
	{
		if((nbytes_temp = write_n_bytes(this, buffer + nbytes_write, BLOCK_SIZE,
				cur_offset)) == -1)
			return -1;
		ALL_ADD_THIRD(cur_offset, nbytes_write, nbytes_temp);
	}
	nbytes_temp = 0;

	//write rest of data
	if(last_nbytes && (nbytes_temp = write_n_bytes(this, buffer + nbytes_write,
			last_nbytes, cur_offset)) == -1)
		return -1;
	ALL_ADD_THIRD(cur_offset, nbytes_write, nbytes_temp);
	//this->f_cur_offset = cur_offset;
	return nbytes_write;
}

void vfs_remove(dataserver_file_t* this)
{
	int i;
	int blocks_count = this->arr_len;

	pthread_mutex_lock(&this->super_block->s_mutex);
	free_blocks(this->super_block, blocks_count, this->f_chunks_arr);
	for(i = 0; i < blocks_count; i++)
		__clear_block_bm(this->super_block->s_block, this->f_blocks_arr[i]);
	this->super_block->s_block->s_last_write_time = time(NULL);
	this->super_block->s_block->s_free_blocks_count += blocks_count;
	pthread_mutex_unlock(&this->super_block->s_mutex);
}
