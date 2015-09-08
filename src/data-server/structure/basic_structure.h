/*
 *basic_structrue.h
 *
 * created on 2015.1.130
 * author:Binyang
 *
 * this file is describe some basic structure in file system
 * our file system only support 4G space(MAX, if block size is 4096). We think block size
 * will large than 4096, so we do not need much room to store this index
 */
#ifndef _BASIC_STRUCTURE_H_
#define _BASIC_STRUCTURE_H_

#include <stdint.h>
#include "global.h"

//sizeof(super_block) = 1028
struct super_block
{
	uint32_t s_dev_num;
	uint32_t s_block_size;
	uint32_t s_blocks_count;
	uint32_t s_free_blocks_count;
	uint32_t s_first_data_block;
	uint16_t s_is_error;
	uint16_t s_status;
	float s_version;
	uint64_t s_mount_time;
	uint64_t s_last_write_time;
//建立一个hash结构来保存chunks到blocks的映射关系，由于动态增长，故保存在内存中，必要情况下可写入磁盘
	uint32_t s_logs_per_group;
	uint32_t s_logs_len;//日志记录长度
	uint32_t s_blocks_per_group;
	uint32_t s_groups_count;//number of group = s_blocks_count / s_blocks_per_group
	uint32_t s_per_group_reserved;//used to store system information
	uint32_t reserved[15];
}super_block;

//多个组可以保存在一个块中，当块大小为4096时，位图可以控制128MB的空间，如此对于一个4G的空间需要32
//个组，当然此时组信息还是可以存储到一个块中。一个块可以控制16G的空间
struct group_desc_block
{
	uint32_t bg_block_bitmap;//块位图的块号
	uint32_t bg_free_blocks_count;
	uint32_t reserved[6];
};

//位图以byte为单位,只有在逻辑上才存在，实际中不需要

typedef struct super_block super_block_t;
typedef struct group_desc_block group_desc_block_t;
char* init_mem_file_system(total_size_t t_size, int dev_num);
//void init_mem_super_block(super_block_t * mem_super_block, int blocks_count, int blocks_per_group, int dev_num);
void free_mem_file_system(char* mem_file_system);
#endif
