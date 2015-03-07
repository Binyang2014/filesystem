/*
 * datasever.h
 *
 * Created on 2015.1.23
 * Author:binyang
 */
#ifndef _FILESYSTEM_DATASERVER_H_
#define _FILESYSTEM_DATASERVER_H_

#include "../structure/vfs_structure.h"

struct data_server_operations
{
	struct dataserver_file* (*open_file)(int* chuncks_arr);
	int (*close_file)(struct dataserver_file*);
	//余下是一堆信息查看和消息通信的函数，暂不理会
};

struct data_server//so far it likes super blocks in VFS, maybe it will be difference
{
	unsigned int d_memory_used;
	unsigned int d_memory_free;
	unsigned int d_memory_used_for_filesystem;
	unsigned int d_memory_free_for_filesystem;
	unsigned int d_network_free;
	struct dataserver_super_block* d_super_block;
	struct list_head* file_list;
	struct data_server_operations* d_op;
	//信号量，必须拥有互斥的信号
};

//define of some function in struct's operations
void init_data_sterver();
void init_lists();
void check_limits();//查看是否还允许分配
int get_current_imformation(struct data_server* server_imf);//返回目前数据节点的信息
void adjust_lists();
int open(char *filename, int mode);
#endif
