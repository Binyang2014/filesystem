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

//数据块结构的结构体
struct data_server
{
	unsigned int memory_used;
	unsigned int memory_free;
	unsigned int memory_used_for_filesystem;
	unsigned int memory_free_for_filesystem;
};
struct chunk
{
	unsigned char is_full;
	unsigned int already_used;
	char* data_chunk;
};

struct chunk_list
{
	struct chunk* next;
};
//根据文件系统的结构建立空闲链表，已使用的链表等等
void init_data_sterver();
void init_lists();
void check_limits();//查看是否还允许分配
void get_current_imformation();
void alloc_a_chunk();
void delete_a_chunk();
void write_chunks();
void read_chunks();
void adjust_lists();
#endif
