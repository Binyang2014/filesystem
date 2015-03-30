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
#include "../../tool/bitmap.h"
#include "../../tool/hash.h"
#include "../../tool/errinfo.h"

//------------------------block operations---------------------------------------------------
//following functions maybe used by read or write function
//we do not want these functions to be see by upper layer because
//we do not let upper layer programmer change super block structure
static void __set_block_bm(super_block_t* super_block, unsigned int block_num)
{
	unsigned int block_num_in_group, blocks_per_group = super_block->s_blocks_per_group;
	unsigned long *bitmap;

	bitmap = __get_bitmap_block(super_block, block_num);
	block_num_in_group = block_num % blocks_per_group;
	bitmap_set(bitmap, block_num_in_group, 1);
}
static void __clear_block_bm(super_block_t* super_block, unsigned int block_num)
{
	unsigned int block_num_in_group, blocks_per_group = super_block->s_blocks_per_group;
	unsigned long *bitmap;

	bitmap = __get_bitmap_block(super_block, block_num);
	block_num_in_group = block_num % blocks_per_group;
	bitmap_clear(bitmap, block_num_in_group, 1);
}
static int __bm_block_set(super_block_t* super_block, unsigned int block_num)
{
	unsigned int block_num_in_group, blocks_per_group = super_block->s_blocks_per_group;
	unsigned long *bitmap;

	bitmap = __get_bitmap_block(super_block, block_num);
	block_num_in_group = block_num % blocks_per_group;
	return bitmap_a_bit_full(bitmap, block_num_in_group);
}
static unsigned int find_first_free_block(super_block_t* super_block, int p_group_id)
{
	unsigned long *bitmap = __get_bitmap_from_gid(super_block, p_group_id);
	unsigned int blocks_per_group = super_block->s_blocks_per_group,
			groups_count = super_block->s_groups_count, block_num_in_group, count = 0;

	//if these group not have a free block, find next group
	while(count < groups_count &&
			(block_num_in_group = find_first_zero_bit(bitmap, blocks_per_group)) == blocks_per_group)
	{
		p_group_id = (p_group_id + 1) % groups_count;
		count++;
	}
	if(count == groups_count)
		return INF_UNSIGNED_INT;
	return block_num_in_group + p_group_id * blocks_per_group;
}
static unsigned int find_next_free_block(super_block_t* super_block, int p_group_id, unsigned int block_num)
{
	unsigned long *bitmap = __get_bitmap_from_gid(super_block, p_group_id);
	unsigned int blocks_per_group = super_block->s_blocks_per_group,
			groups_count = super_block->s_groups_count,
			block_num_in_group = block_num % super_block->s_blocks_per_group;

	//can't find free block in this group, go to next group and find first free block
	if((block_num_in_group = find_next_zero_bit(bitmap, blocks_per_group, block_num_in_group))
			== blocks_per_group)
		return find_first_free_block(super_block, p_group_id + 1);
	return block_num_in_group + p_group_id * blocks_per_group;
}
//static void sb_regist_block(int chunk_num, int block_num);
//static void sb_logout_block(int chunk_num);

static char* find_a_block(dataserver_sb_t* dataserver_sb, unsigned int block_num)
{
	unsigned int blocks_per_groups;
	int group_offset;
	super_block_t *super_block = dataserver_sb->s_block;
	char* block;
	blocks_per_groups = dataserver_sb->s_op->get_blocks_per_groups;
	group_offset = block_num % blocks_per_groups;
	if((group_offset + 1)  <= dataserver_sb->s_op->get_per_group_reserved)
	{
		fprintf(stderr, "can not read reserved information\n");
		return NULL;
	}
	//may be we should check if this block's bitmap is set, we are not trust server
	block = (char* )super_block + block_num * BLOCK_SIZE;
	return block;
}

//----------------------super block operations----------------------------------------
unsigned int get_blocks_count(dataserver_sb_t* this)
{
	return this->s_block->s_blocks_count;
}

unsigned int get_free_blocks_count(dataserver_sb_t* this)
{
	return this->s_block->s_free_blocks_count;
}

unsigned int get_blocks_per_groups(dataserver_sb_t* this)
{
	return this->s_block->s_blocks_per_group;
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

unsigned int get_per_group_reserved(dataserver_sb_t* this)
{
	return this->s_block->s_per_group_reserved;
}

//----------------------------------------------------------------------------------
//给定chunk号，通过hash函数查找相应的block number号
unsigned int find_a_block_num(dataserver_sb_t* this, unsigned long long chunk_num)
{
	char str[MAX_NUM_SIZE + 1];
	unsigned int hash_num;
	ulltoa(chunk_num, str);
	hash_num = this->hash_function(str, this->s_hash_table->hash_table_size);

	//Programmer should make sure that at least a value in block_arr is -1
	while(this->s_hash_table->blocks_arr[hash_num] != INF_UNSIGNED_INT &&
			this->s_hash_table->chunks_arr[hash_num] != chunk_num)
	{
		hash_num = (hash_num + 1) % (this->s_hash_table->hash_table_size);
	}

	if(this->s_hash_table->chunks_arr[hash_num] == chunk_num)
		return this->s_hash_table->blocks_arr[hash_num];
#ifdef DEBUG
	fprintf(stderr, "Can not find certain chunk_num in this hash table\n");
#endif
	return INF_UNSIGNED_INT;
}
static unsigned int find_a_block_with_hash_num(dataserver_sb_t* this, unsigned long long chunk_num,
		unsigned int* t_hash_num)
{
	char str[MAX_NUM_SIZE + 1];
	unsigned int hash_num = INF_UNSIGNED_INT;
	ulltoa(chunk_num, str);
	hash_num = this->hash_function(str, this->s_hash_table->hash_table_size);
	//Programmer should make sure that at least a value in block_arr is -1
	while(this->s_hash_table->blocks_arr[hash_num] != INF_UNSIGNED_INT &&
			this->s_hash_table->chunks_arr[hash_num] != chunk_num)
	{
		hash_num = (hash_num + 1) % (this->s_hash_table->hash_table_size);
	}

	if(this->s_hash_table->chunks_arr[hash_num] == chunk_num)
	{
		*t_hash_num = hash_num;
		return this->s_hash_table->blocks_arr[hash_num];
	}

	fprintf(stderr, "Can not find certain chunk_num in this hash table\n");
	return INF_UNSIGNED_INT;
}

//success return address of array fail return null
unsigned int* find_serials_blocks(dataserver_sb_t* this, int arr_size,
			unsigned long long* chunks_arr, unsigned int* blocks_arr)
{
	int i;
	unsigned int t_block_number;

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
//the block number
unsigned int alloc_a_block(dataserver_sb_t* this, unsigned long long chunk_num, unsigned int block_num)
{
	unsigned int hash_num;
	char str[MAX_NUM_SIZE + 1];
	ulltoa(chunk_num, str);
	hash_num = this->hash_function(str, this->s_hash_table->hash_table_size);

	while(this->s_hash_table->blocks_arr[hash_num] != INF_UNSIGNED_INT)
		hash_num = (hash_num + 1) % this->s_hash_table->hash_table_size;
	if(this->s_hash_table->blocks_arr[hash_num] == INF_UNSIGNED_INT)
	{
		this->s_hash_table->blocks_arr[hash_num] = block_num;
		this->s_hash_table->chunks_arr[hash_num] = chunk_num;
		return hash_num;
	}

	fprintf(stderr, "Can not allocated a block in this file system\n");
	return INF_UNSIGNED_INT;
}

unsigned int* alloc_blocks_with_hash(dataserver_sb_t* this, int arr_size,
			unsigned long long* chunks_arr, unsigned int* blocks_arr, unsigned int* hash_arr)
{
	int i;
	unsigned int t_hash_num;
	for(i = 0; i < arr_size; i++)
	{
		t_hash_num = alloc_a_block(this, chunks_arr[i], blocks_arr[i]);
		if(t_hash_num != INF_UNSIGNED_INT)
			hash_arr[i] = t_hash_num;
		else
			return NULL;
	}
	return hash_arr;
}

int alloc_blocks(dataserver_sb_t* this, int arr_size,
			unsigned long long* chunks_arr, unsigned int* blocks_arr)
{
	int i;
	unsigned int t_hash_num;
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
unsigned int free_a_block(dataserver_sb_t* this, unsigned long long chunk_num)
{
	unsigned int block_num, hash_num;
	block_num = find_a_block_with_hash_num(this, chunk_num, &hash_num);
	if(block_num == INF_UNSIGNED_INT)
	{
		fprintf(stderr, "free a block fails\n");
		return INF_UNSIGNED_INT;
	}
	this->s_hash_table->blocks_arr[hash_num] = INF_UNSIGNED_INT;
	this->s_hash_table->chunks_arr[hash_num] = INF_UNSIGNED_LONG;
	return block_num;
}

unsigned int* free_blocks_with_return(dataserver_sb_t* this, int arr_size,
		unsigned long long* chunks_arr, unsigned int* blocks_arr)
{
	int i;
	unsigned int t_block_num;
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

int free_blocks(dataserver_sb_t* this, int arr_size, unsigned long long* chunks_arr)
{
	int i;
	unsigned int t_block_num;
	for(i = 0; i < arr_size; i++)		{
		t_block_num = free_a_block(this, chunks_arr[i]);
		if(t_block_num != INF_UNSIGNED_INT)
			continue;
		else
			return INF_UNSIGNED_INT;
	}
	return 0;
}
//---------------------------------------------------------------------------------

void print_sb_imf(dataserver_sb_t* this)
{
	unsigned int blocks_count = get_blocks_count(this);
	unsigned int free_blocks_count = get_free_blocks_count(this);
	unsigned int blocks_per_group = get_blocks_per_groups(this);
	float filesystem_version = get_filesystem_version(this);
	unsigned int groups_conut = get_groups_conut(this);
	time_t last_write_time = get_last_write_time(this);
	unsigned short status = get_superblock_status(this);
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

static int read_a_block(dataserver_file_t *this, char* buffer, off_t offset)
{
	unsigned int *blocks_arr;
	unsigned int block_num;

	blocks_arr = this->f_blocks_arr;
	block_num = blocks_arr[offset / BLOCK_SIZE];
	if(! __bm_block_set(this->super_block->s_block, block_num))
	{
		fprintf(stderr, "You want to read a block that have no data in it\n");
		return -1;
	}
	memcpy(buffer,find_a_block(this->super_block, block_num), BLOCK_SIZE);
	return BLOCK_SIZE;
}

static int read_rest_bytes(dataserver_file_t *this, char* buffer, int nbytes, off_t offset)
{
	unsigned int *blocks_arr;
	unsigned int block_num;

	blocks_arr = this->f_blocks_arr;
	block_num = blocks_arr[offset / BLOCK_SIZE];
	if(! __bm_block_set(this->super_block->s_block, block_num))
	{
		fprintf(stderr, "You want to read a block that have no data in it\n");
		return -1;
	}
	memcpy(buffer, find_a_block(this->super_block, block_num), nbytes);
	return nbytes;
}

int vfs_read(dataserver_file_t *this, char* buffer, size_t count, off_t offset)
{
	int nblocks, last_nbytes, first_nbytes;
	int nbytes_read = 0, nbytes_temp;
	off_t cur_offset, end_offset;
	int i;

	cur_offset = offset;//set current offset to offset
	end_offset = offset + count;

	first_nbytes = cal_first_bytes(offset);
	//if need data only in one block
	if(count < first_nbytes)
	{
		first_nbytes = count;
		if((nbytes_temp = read_rest_bytes(this, buffer + nbytes_read, first_nbytes, cur_offset)) == -1)
			return -1;
		ALL_ADD_THIRD(cur_offset, nbytes_read, nbytes_temp);
		this->f_cur_offset = cur_offset;
		return nbytes_read;
	}

	//read first bytes
	if((nbytes_temp = read_rest_bytes(this, buffer + nbytes_read, first_nbytes, cur_offset)) == -1)
		return -1;
	ALL_ADD_THIRD(cur_offset, nbytes_read, nbytes_temp);

	//if need data across tow blocks but not any of them is filled
	if (cur_offset + BLOCK_SIZE > end_offset)
	{
		last_nbytes = end_offset - cur_offset;
		if((nbytes_temp = read_rest_bytes(this, buffer + nbytes_read, last_nbytes, cur_offset)) == -1)
			return -1;
		ALL_ADD_THIRD(cur_offset, nbytes_read, nbytes_temp);
		this->f_cur_offset = cur_offset;
		return nbytes_read;
	}

	//read blocks of data
	nblocks = (count - first_nbytes) / BLOCK_SIZE;
	last_nbytes = (count - first_nbytes) % BLOCK_SIZE;

	for (i = 0; i < nblocks; i++)
	{
		if((nbytes_temp = read_a_block(this, buffer + nbytes_read, cur_offset)) == -1)
			return -1;
		ALL_ADD_THIRD(cur_offset, nbytes_read, nbytes_temp);
	}

	//read rest of data
	if((nbytes_temp = read_rest_bytes(this, buffer + nbytes_read, last_nbytes, cur_offset)) == -1)
		return -1;
	ALL_ADD_THIRD(cur_offset, nbytes_read, nbytes_temp);
	this->f_cur_offset = cur_offset;
	return nbytes_read;
}

static int write_rest_bytes(dataserver_file_t *this, char* buffer, int nbytes, off_t offset)
{
	unsigned int *blocks_arr;
	unsigned long long* chunks_arr;
	unsigned int block_num;
	unsigned long long chunk_num;

	chunks_arr = this->f_chunks_arr;
	blocks_arr = this->f_blocks_arr;
	chunk_num = chunks_arr[offset / BLOCK_SIZE];
	//if this file already has this block
	if((block_num = this->super_block->s_op->find_a_block_num(this->super_block, chunk_num))
			!= INF_UNSIGNED_INT)
	{
#ifdef DEBUG
		if( !__bm_block_set(this->super_block->s_block, block_num))
			err_quit("You use a block but not set the bitmap");
#endif
		this->f_blocks_arr[offset / BLOCK_SIZE] = block_num;//this statement may be not useful
		memcpy(find_a_block(this->super_block, block_num), buffer, nbytes);
		return nbytes;
	}

	pthread_mutex_lock(this->super_block->s_mutex);
	//here we find a free block from very beginning, may be we want find it near other block in
	//this file, it will be considered later
	if( (block_num = find_first_free_block(this->super_block->s_block, 0)) == INF_UNSIGNED_INT )
	{
		err_msg("this data server is full");
		return -1;
	}
	memcpy(find_a_block(this->super_block, block_num), buffer, nbytes);
	__set_block_bm(this->super_block->s_block, block_num);
	if( alloc_a_block(this->super_block, chunk_num, block_num) == INF_UNSIGNED_INT)
	{
		err_msg("wrong in allocated a block in hash table");
		pthread_mutex_unlock(this->super_block->s_mutex);
		return -1;
	}
	pthread_mutex_unlock(this->super_block->s_mutex);
	return nbytes;
}

int vfs_write(dataserver_file_t* this, char* buffer, size_t count, off_t offset)
{
	int nblocks, last_nbytes, first_nbytes;
	int nbytes_write = 0, nbytes_temp;
	off_t cur_offset, end_offset;
	int i;

	cur_offset = offset;
	end_offset = offset + count;
	first_nbytes = cal_first_bytes(offset);

	//only need to write data to one block
	if(count < first_nbytes)
	{
		first_nbytes = count;
		if((nbytes_temp = write_rest_bytes(this, buffer + nbytes_write, first_nbytes, cur_offset)) == -1)
				return -1;
		ALL_ADD_THIRD(cur_offset, nbytes_write, nbytes_temp);
		this->f_cur_offset = cur_offset;
		return nbytes_write;
	}
	//...
	return 0;
}

