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

#define MAX_ALLOC_SIZE 1<<30//这部分应该在配置文件中配置
#define BLOCK_SIZE (1<<12)
#define N_BLOCKS 14//建立chunck号和block号的对应关系索引
//sizeof(super_blocl) = 1028
struct super_block
{
	unsigned int s_dev_num;
	unsigned int s_block_size;
	unsigned int s_blocks_count;
	unsigned int s_free_blocks_count;
	unsigned int s_first_data_block;
	unsigned short s_is_error;
	unsigned short s_status;
	unsigned int s_version;
	unsigned long s_mount_time;
	unsigned long s_last_write_time;
	//下面部分需要重新设计
//	unsigned int s_direct_blocks[N_BLOCKS];//保存指向数据块的指针,即块号
//	unsigned int s_first_blocks[N_BLOCKS];//间接索引节点表
//	unsigned int s_second_block[N_BLOCKS];//二次间接索引节点表
//	unsigned int s_in_sec_block[N_BLOCKS][N_BLOCKS];//二次间接索引表包含的内容，指向实际的块号
//建立一个hash结构来保存chunks到blocks的映射关系，由于动态增长，故保存在内存中，必要情况下可写入磁盘

	unsigned int logs_len;//日志记录长度
	unsigned int s_blocks_per_group;
	unsigned int s_groups_count;//number of group = s_blocks_count / s_blocks_per_group
	unsigned int reserved[8];
}super_block;

//多个组可以保存在一个块中，当块大小为4096时，位图可以控制128MB的空间，如此对于一个4G的空间需要32
//个组，当然此时组信息还是可以存储到一个块中。一个块可以控制16G的空间
struct group_desc_block
{
	unsigned int bg_block_bitmap;//块位图的块号
	unsigned int bg_free_blocks_count;
	unsigned int reserved[6];
};

//位图以byte为单位,只有在逻辑上才存在，实际中不需要
//struct bitmap
//{
//	char bitmap[BLOCK_SIZE];
//};

//4 blocks used to store logs
//struct log_block
//{
//	char log_blocks[4][BLOCK_SIZE];
//};

enum TOTAL_SIZE
{
	SMALLEST = 24,	//16MB
	SMALL = 26,		//64MB
	MIDDLE = 28,	//256MB
	LARGE = 30,		//1GB
	LARGEST = 32	//4GB
};

typedef enum TOTAL_SIZE total_size_t;
typedef struct super_block super_block_t;
typedef struct group_desc_block group_desc_block_t;
char* init_mem_file_system(total_size_t t_size, int dev_num);
void init_mem_super_block(super_block_t * mem_super_block, int blocks_count, int blocks_per_group, int dev_num);
#endif
