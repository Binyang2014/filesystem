/**
 * This test file is to make true that vfs system works well
 *
 */
#include "vfs_structure.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned long long chunks_arr[10] = {0x345, 0xfff, 0x123456, 0x19203454, 0x12343, 0x12438959, 0x11111111, 0x2222222222222222,
		0x1243576, 0xff32};
static char write_buff[4200] = {0};
static char read_buff[4200] = {0};
vfs_hashtable_t* init_hashtable(int hash_len)
{
	vfs_hashtable_t* hash_table = (vfs_hashtable_t* )malloc(sizeof(vfs_hashtable_t));
	hash_table->hash_table_size = hash_len;
	hash_table->chunks_arr = (unsigned long long* )malloc(sizeof(unsigned long long) * hash_len);
	hash_table->blocks_arr = (unsigned int* )malloc(sizeof(unsigned int) * hash_len);
	return hash_table;
}

void s_hash_test(dataserver_sb_t* d_sb)
{
	unsigned int block_num = 255, hash_num;
	size_t chunk_num = 12;
	int i;

	unsigned int blocks_arr[10] = {12, 6543, 432657, 34, 1234, 4356, 1904, 8192, 7935, 23876};
	unsigned int blocks_arr_t[10];
	unsigned int* ans;

	//I will test if hash table works well
	hash_num = d_sb->s_op->alloc_a_block(d_sb, chunk_num, block_num);
	printf("hash number is %u\n", hash_num);
	hash_num = d_sb->s_op->find_a_block_num(d_sb, chunk_num);
	printf("block number is %u\n", hash_num);
	hash_num = d_sb->s_op->free_a_block(d_sb, chunk_num);
	printf("block number is %u\n", hash_num);
	hash_num = d_sb->s_op->find_a_block_num(d_sb, chunk_num);
	printf("block number is %u\n", hash_num);

	d_sb->s_op->alloc_blocks(d_sb, 10, chunks_arr, blocks_arr);
	d_sb->s_op->find_serials_blocks(d_sb, 10, chunks_arr, blocks_arr_t);
	for(i = 0; i < 10; i++)
		printf("%u\t", blocks_arr_t[i]);
	printf("\n");

	d_sb->s_op->free_blocks(d_sb, 10, chunks_arr);
	ans = d_sb->s_op->find_serials_blocks(d_sb, 10, chunks_arr, blocks_arr_t);
	if(ans != NULL)
	{
		for(i = 0; i < 10; i++)
			printf("%u\t", blocks_arr_t[i]);
		printf("\n");
	}
}

void vfs_basic_read_write_test(dataserver_file_t* d_file)
{
	int i, arr_len;
	unsigned int offset;
	arr_len = d_file->arr_len;
	for(i = 0; i < arr_len; i++)
		d_file->f_chunks_arr[i] = chunks_arr[i];

	printf("write to write buffer\n");
	for(i = 0; i < 10; i++)
	{
		write_buff[i] = '0' + i;
		printf("%c ", write_buff[i]);
	}
	memcpy(write_buff + BLOCK_SIZE - 1, write_buff, 10);
	printf("\n");

	offset = 2;

	//write to one block with offset
	printf("Test read and write by writing to one block with offset\n");
	d_file->f_op->vfs_write(d_file, write_buff, 7, offset);
	d_file->f_op->vfs_read(d_file, read_buff, 7, offset);
	printf("read from blocks\n");
	for(i = 0; i < 7; i++)
	{
		printf("%c ", read_buff[i]);
	}
	memset(read_buff, 0 ,sizeof(read_buff));
	printf("\n-----------------------------------------------------------------\n");

	printf("Test read and write by writing across two blocks\n");
	d_file->f_op->vfs_write(d_file, write_buff, 7, BLOCK_SIZE -2);
	d_file->f_op->vfs_read(d_file, read_buff, 7, BLOCK_SIZE -2);
	for(i = 0; i < 7; i++)
	{
		printf("%c ", read_buff[i]);
	}
	memset(read_buff, 0 ,sizeof(read_buff));
	printf("\n------------------------------------------------------------------\n");

	printf("Test read and write by writing to many blocks with offset\n");
	d_file->f_op->vfs_write(d_file, write_buff, 5+BLOCK_SIZE, BLOCK_SIZE -2);
	d_file->f_op->vfs_read(d_file, read_buff, 5+BLOCK_SIZE, BLOCK_SIZE -2);
	for(i = 0; i < 10; i++)
	{
		printf("%c ", read_buff[i]);
	}
	for(i = 0; i < 10; i++)
	{
		printf("%c ", (read_buff + BLOCK_SIZE - 1)[i]);
	}
	printf("\nfinish basic read write test!!!\n");
}

int main()
{
	dataserver_sb_t* d_superblock;
	dataserver_file_t* d_file;
	vfs_hashtable_t* f_arr;
	int f_arr_len;

	d_superblock = vfs_init(MIDDLE, 1);
	d_superblock->s_op->print_sb_imf(d_superblock);
	s_hash_test(d_superblock);
	printf("------------------------------------------------\n\n");

	printf("-------------test about file operations------------\n");
	//init structure of data server file
	d_file = (dataserver_file_t* )malloc(sizeof(dataserver_file_t));
	f_arr_len = 8;

	f_arr = init_hashtable(f_arr_len);
	d_file = init_vfs_file(d_superblock, d_file, f_arr, VFS_WRITE);

	vfs_basic_read_write_test(d_file);
	return 0;
}
