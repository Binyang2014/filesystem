/**
 * This test file is to make true that vfs system works well
 *
 */
#include "vfs_structure.h"
#include <stdio.h>

int main()
{
	char* filesystem;
	dataserver_sb_t* d_superblock;
	unsigned int block_num = 255, hash_num;
	size_t chunk_num = 12;
	int i;

	unsigned long long chunks_arr[10] = {0x345, 0xfff, 0x123456, 0x19203454, 0x12343, 0x12438959, 0x11111111, 0x2222222222222222,
			0x1243576, 0xff32};
	unsigned int blocks_arr[10] = {12, 6543, 432657, 34, 1234, 4356, 1904, 8192, 7935, 23876};
	unsigned int blocks_arr_t[10];
	unsigned int hash_arr[10];
	unsigned int* ans;

	filesystem = init_mem_file_system(MIDDLE, 1);
	d_superblock = init_vfs_sb(filesystem);
	d_superblock->s_op->print_sb_imf(d_superblock);

	//I will test if hash table works well
	hash_num = d_superblock->s_op->alloc_a_block(d_superblock, chunk_num, block_num);
	printf("block number is %u\n", hash_num);
	hash_num = d_superblock->s_op->find_a_block_num(d_superblock, chunk_num);
	printf("block number is %u\n", hash_num);
	hash_num = d_superblock->s_op->free_a_block(d_superblock, chunk_num);
	printf("block number is %u\n", hash_num);
	hash_num = d_superblock->s_op->find_a_block_num(d_superblock, chunk_num);
	printf("block number is %u\n", hash_num);

	d_superblock->s_op->alloc_blocks(d_superblock, 10, chunks_arr, blocks_arr);
	//	for(i = 0; i < 10; i++)
//	d_superblock->s_op->alloc_blocks_with_hash(d_superblock, 10, chunks_arr, blocks_arr, hash_arr);
//	for(i = 0; i < 10; i++)
//		printf("%u\t", hash_arr[i]);
//	printf("\n");
	d_superblock->s_op->find_serials_blocks(d_superblock, 10, chunks_arr, blocks_arr_t);
	for(i = 0; i < 10; i++)
		printf("%u\t", blocks_arr_t[i]);
	printf("\n");
	d_superblock->s_op->free_blocks(d_superblock, 10, chunks_arr);
//	d_superblock->s_op->free_blocks_with_return(d_superblock, 10, chunks_arr, blocks_arr);
//	for(i = 0; i < 10; i++)
//		printf("%u\t", blocks_arr[i]);
//	printf("\n");
	ans = d_superblock->s_op->find_serials_blocks(d_superblock, 10, chunks_arr, blocks_arr_t);
	if(ans != NULL)
	{
		for(i = 0; i < 10; i++)
			printf("%u\t", blocks_arr_t[i]);
		printf("\n");
	}
	return 0;
}
