/*
 *basic_structrue.h
 *
 * created on 2015.1.130
 * author:Binyang
 * this file is describe some basic structure in file system
 */
#ifndef _BASIC_STRUCTURE_H_
#define _BASIC_STRUCTURE_H_

#include <pthread.h>

#define MAX_ALLOC_SIZE 1<<30//这部分应该在配置文件中配置
#define BLOCK_SIZE (1<<20)
#define N_BLOCKS 14//建立chunck号和block号的对应关系索引
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
	unsigned int s_mount_time;
	unsigned int s_last_write_time;
	unsigned int s_direct_blocks[N_BLOCKS];//保存指向数据块的指针,即块号
	unsigned int s_first_blocks[N_BLOCKS];//间接索引节点表
	unsigned int s_second_block[N_BLOCKS];//二次间接索引节点表
	unsigned int s_in_sec_block[N_BLOCKS][N_BLOCKS];//二次间接索引表包含的内容，指向实际的块号
	unsigned int reserved[9];
}super_block;

//位图以byte为单位
char bitmap[BLOCK_SIZE];


//below is logic structure

struct super_block_operations
{
	pthread_mutex_t s_mutex;
	unsigned int (*get_blocks_count)();
	unsigned int (*get_free_block_count)();
	unsigned int (*get_filesystem_version)();
	time_t (*get_last_write_time)();
	unsigned int(*find_a_block_num)(unsigned int chunk_num);
};

struct file_operations
{
	int (*read)(unsigned int start_pos, unsigned int len);
	int (*open)(char *filename, int mode);
};

//only one copy in memory
struct datasetver_super_block
{
	struct super_block_operations *s_op;
};

//once open a file, this structure should be built
struct dataserver_file//I think we need a file structure to point opening files
{
	struct dataserver_file* head_list;//应该有个双向循环链表
	char file_name[256];
	unsigned int cur_off_side;
	unsigned int file_len;
	struct file_operations *f_op;
};

#endif
