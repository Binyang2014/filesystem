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
	filesystem = init_mem_file_system(MIDDLE, 1);
	d_superblock = init_vfs_sb(filesystem);
	d_superblock->s_op->print_sb_imf(d_superblock);

	//I will test if hash table works well
	hash_num = d_superblock->s_op->alloc_a_block(d_superblock, chunk_num, block_num);
	printf("block number is %u\n", hash_num);
	hash_num = d_superblock->s_op->find_a_block_num(d_superblock, chunk_num, block_num);
	printf("block number is %u\n", hash_num);
	hash_num = d_superblock->s_op->free_a_block(d_superblock, chunk_num, block_num);
	printf("block number is %u\n", hash_num);
	hash_num = d_superblock->s_op->find_a_block_num(d_superblock, chunk_num, block_num);
	printf("block number is %u\n", hash_num);
	return 0;
}
