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
//数据块结构的结构体
typedef struct data_server
{
	unsigned int memory_used;
	unsigned int memory_free;
	unsigned int memory_used_for_filesystem;
	unsigned int memory_free_for_filesystem;
}data_server;

typedef struct chunk
{
	unsigned int chunk_num;
	unsigned char is_full;
	unsigned int already_used;
	char* data_chunk;
}chunk;

typedef struct chunk_list
{
	struct chunk* p_chunk;//point to real block
	struct chunk* next;
}chunk_list;

typedef struct super_block
{
	;
}super_block;

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
