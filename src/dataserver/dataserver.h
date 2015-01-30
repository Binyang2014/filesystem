/*
 * datasever.h
 *
 * Created on 2015.1.23
 * Author:binyang
 */
#ifndef _FILESYSTEM_DATASERVER_H_
#define _FILESYSTEM_DATASERVER_H_

#include "type/basic_structure.h"

struct data_server_operations
{
	;
};

struct data_server//so far it likes super blocks in VFS, maybe it will be difference
{
	unsigned int memory_used;
	unsigned int memory_free;
	unsigned int memory_used_for_filesystem;
	unsigned int memory_free_for_filesystem;
	unsigned int network_free;
	struct data_server_operations *d_op;
	//信号量，必须拥有互斥的信号
};

//define of some function in struct's operations
void init_data_sterver();
void init_lists();
void check_limits();//查看是否还允许分配
int get_current_imformation(struct data_server * server_imf);//返回目前数据节点的信息
void alloc_a_chunk();
void delete_a_chunk();
void write_chunks();
void read_chunks();
void adjust_lists();
int read(unsigned int start_pos, unsigned int len);
int open(char *filename, int mode);
#endif
