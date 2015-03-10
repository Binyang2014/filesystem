/*
 * create on 2015.1.31
 * author: Binyang
 *
 * to complete functions
 */
#include "basic_structure.h"
#include "../../tool/bitmap.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>

static void init_mem_super_block(super_block_t * mem_super_block, int blocks_count, int blocks_per_group, int dev_num)
{
	mem_super_block->s_block_size = BLOCK_SIZE;
	mem_super_block->s_blocks_count = blocks_count;
	mem_super_block->s_blocks_per_group = blocks_per_group;
	mem_super_block->s_groups_count = blocks_count / blocks_per_group;
	mem_super_block->s_dev_num = dev_num;
	mem_super_block->s_first_data_block = 0;
	mem_super_block->s_logs_per_group = N_LOG_BLOCKS_PER_G;
	mem_super_block->s_logs_len = 0;
	mem_super_block->s_free_blocks_count = blocks_count -
			(1 + 1 + 1 + mem_super_block->s_logs_per_group) * (blocks_count / blocks_per_group);
	//1 for super block 1 for group descibe 1 for bit map 4 for logs
	mem_super_block->s_per_group_reserved = 1 + 1 + 1 + mem_super_block->s_logs_per_group;
	mem_super_block->s_is_error = 0;
	mem_super_block->s_last_write_time = time(NULL);
	mem_super_block->s_mount_time = time(NULL);
	mem_super_block->s_status = 0;
	mem_super_block->s_version = 0.1;
	memset(mem_super_block->reserved, 0, sizeof(mem_super_block->reserved));
}

char* init_mem_file_system(total_size_t t_size, int dev_num)
{
	char * mem_file_system;
	char *cur_mem_pos;
	int blocks_count, blocks_per_group, groups_count, first_bitmap_pos = 2;
	super_block_t *mem_super_block;
	group_desc_block_t *mem_group_desc_block;
	size_t total_size;
	int i;

	total_size = 1 << t_size;
	if(total_size > MAX_ALLOC_SIZE)
	{
		fprintf(stderr, "exceed max size");
		return NULL;
	}
	mem_file_system = (char *)malloc(total_size);
	if(mem_file_system == NULL)
	{
		perror("can not allocate memory for file system");
		return NULL;
	}

	blocks_count = total_size / BLOCK_SIZE;
	blocks_per_group = BLOCK_SIZE * 8;//each byte is 8 bit
	if( (blocks_count % blocks_per_group) )//力求平分
	{
		groups_count = blocks_count / blocks_per_group + 1;
		blocks_per_group = blocks_count / groups_count;
		if(blocks_count % groups_count)
		{
			fprintf(stderr, "every group need equal blocks\n");
			free(mem_file_system);
			return NULL;
		}
	}
	else
		groups_count = blocks_count / blocks_per_group;

	if(groups_count * sizeof(group_desc_block_t) > BLOCK_SIZE)//组描述块必须只有一个
	{
		fprintf(stderr, "can not support this condition\n");
		free(mem_file_system);
		return NULL;
	}

	cur_mem_pos = mem_file_system;

	//format file system
	mem_super_block = (super_block_t *)cur_mem_pos;
	init_mem_super_block(mem_super_block, blocks_count, blocks_per_group, dev_num);
	cur_mem_pos = cur_mem_pos + BLOCK_SIZE;
	for(i = 0; i < groups_count; i++)
	{
		mem_group_desc_block = (group_desc_block_t*)cur_mem_pos;
		mem_group_desc_block->bg_block_bitmap = first_bitmap_pos + i * blocks_per_group;
		mem_group_desc_block->bg_free_blocks_count = blocks_per_group - 1 - 1 - 1 - N_LOG_BLOCKS_PER_G;
		cur_mem_pos = cur_mem_pos + sizeof(group_desc_block_t);
	}
	cur_mem_pos = cur_mem_pos - groups_count * sizeof(group_desc_block_t) + BLOCK_SIZE;
	bitmap_zero((unsigned long*)cur_mem_pos, blocks_per_group);
	bitmap_set((unsigned long*)cur_mem_pos, 0, 7);

	cur_mem_pos = mem_file_system;
	for(i = 1; i < groups_count; i++)
	{
		memcpy(cur_mem_pos + BLOCK_SIZE * blocks_per_group, cur_mem_pos, 3 * BLOCK_SIZE);
		cur_mem_pos = cur_mem_pos + BLOCK_SIZE * blocks_per_group;
	}

	return mem_file_system;
}
