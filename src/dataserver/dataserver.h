/*
 * datasever.h
 *
 * Created on 2015.1.23
 * Author:binyang
 */

#ifndef _FILESYSTEM_CLIENT_DATASERVER_H_
#define _FILESYSTEM_CLIENT_DATASERVER_H_
#define MAX_ALLOC_SIZE 10000//这部分应该在配置文件中配置
#define CHUNK_SIZE (1<<20)
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <>
//#include <pthread.h>

//数据块结构的结构体，用来记录数据节点的整体信息
typedef struct data_server
{
	unsigned int memory_used;
	unsigned int memory_free;
	unsigned int memory_used_for_filesystem;
	unsigned int memory_free_for_filesystem;
	unsigned int network_free;
}data_server;

/***
 * 由于文件中提供了文件的大小和所用的chunk_id这里我们不再提供chunk是否用过和是否使用满的
 * 信息，是否用过的信息在位图中给出。
 */
typedef struct chunk
{
	unsigned int chunk_id;//用于表示文件中的chunk号
	unsigned int block_id;//用于表示文件系统分块的block号
	char* data_chunk;//point to real block
	int is_locked;
	//pthread_mutex_t lock;
}chunk;

/*typedef struct chunk_list
{
	struct chunk* p_chunk;//point to real block
	struct chunk* next;
	struct chunk* pre;
}chunk_list;*/

typedef struct super_block
{
	unsigned int block_size;
	unsigned int total_blocks;
	unsigned int num_free_blocks;
	unsigned int first_index_for_block;
	unsigned short is_error;
	unsigned short status;
	char is_locked;
	//pthread_mutex_t lock;
	time_t last_write_time;
	//int free_block_array[];
}super_block;

//位图
char bitmap[100][100];

//根据文件系统的结构建立空闲链表，已使用的链表等等
void init_data_sterver();
void init_lists();
void check_limits();//查看是否还允许分配
int get_current_imformation(data_server * server_imf);//返回目前数据节点的信息
void alloc_a_chunk();
void delete_a_chunk();
void write_chunks();
void read_chunks();
void adjust_lists();
#endif
