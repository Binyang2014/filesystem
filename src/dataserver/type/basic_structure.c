/*
 * create on 2015.1.31
 * author: Binyang
 *
 * to complete functions
 */
#include "basic_structure.h"
#include "../../tool/bimap.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

void init_mem_super_block(super_block_t * mem_super_block, int blocks_count, int blocks_per_group, int dev_num)
{
	mem_super_block->s_block_size = BLOCK_SIZE;
	mem_super_block->s_blocks_count = blocks_count;
	mem_super_block->s_blocks_per_group = blocks_per_group;
	mem_super_block->s_dev_num = dev_num;
	mem_super_block->s_first_data_block = 0;
	mem_super_block->s_free_blocks_count = blocks_count - (1 - 1 - 1 - 4) * (blocks_count / blocks_per_group);
	//1 for super block 1 for group descibe 1 for bit map 4 for logs
	mem_super_block->s_is_error = 0;
	mem_super_block->s_last_write_time = time(NULL);
	mem_super_block->s_mount_time = time(NULL);
	mem_super_block->s_status = 0;
	mem_super_block->s_version = 0.1;
}

char* init_mem_file_system(unsigned int total_size, int dev_num)
{
	char *const mem_file_system;
	char *cur_mem_pos;
	int blocks_count, blocks_per_group, groups_count, first_bitmap_pos = 2, last_group_size;
	super_block_t *mem_super_block;
	group_desc_block_t *mem_group_desc_block;
	int i;

	if(total_size > MAX_ALLOC_SIZE)
	{
		fprintf(stderr, "exceed max size");
		return NULL;
	}
	mem_file_system = (char *)malloc(total_size);
	if(mem_file_system)
	{
		perror("can not allocate memory for file system");
		return NULL;
	}

	blocks_count = total_size / BLOCK_SIZE;
	blocks_per_group = BLOCK_SIZE * 8;
	if( (total_size % blocks_per_group) )//力求平分
	{
		groups_count = total_size / blocks_per_group + 1;
		blocks_per_group = total_size / groups_count;
	}
	else
		groups_count = total_size / blocks_per_group;

	if(groups_count * sizeof(group_desc_block_t) > BLOCK_SIZE)//组描述块必须只有一个
	{
		fprintf(stderr, "can not support this condition");
		return NULL;
	}

	cur_mem_pos = mem_file_system;

	mem_super_block = (super_block_t *)cur_mem_pos;
	init_mem_super_block(mem_super_block, blocks_count, blocks_per_group, dev_num);
	cur_mem_pos = cur_mem_pos + BLOCK_SIZE;
	for(i = 0; i < groups_count; i++)
	{
		mem_group_desc_block = (group_desc_block_t*)cur_mem_pos;
		mem_group_desc_block->bg_block_bitmap = first_bitmap_pos + i * blocks_per_group;
		mem_group_desc_block->bg_free_blocks_count = blocks_per_group - 1 - 1 - 1 - 4;
		cur_mem_pos = cur_mem_pos + sizeof(group_desc_block_t);
	}
	cur_mem_pos = cur_mem_pos - groups_count * sizeof(group_desc_block_t) + BLOCK_SIZE;
	bitmap_zero((unsigned long*)cur_mem_pos, blocks_per_group - 1 - 1 - 1 - 4);

	cur_mem_pos = mem_file_system;
	for(i = 0; i < groups_count; i++)
	{
		memcpy(cur_mem_pos + BLOCK_SIZE * blocks_per_group, cur_mem_pos, 3 * BLOCK_SIZE);
	}

	return mem_file_system;
}
