/*
 * datasever.h
 *
 * Created on 2015.1.23
 * Author:binyang
 */
#ifndef _FILESYSTEM_DATASERVER_H_
#define _FILESYSTEM_DATASERVER_H_

#include <pthread.h>
#include "../structure/vfs_structure.h"
#include "../../tool/message.h"
#include "dataserver_comm.h"

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
	unsigned int machine_id;

	dataserver_sb_t* d_super_block;
	struct data_server_operations* d_op;

	//buffers
	dataserver_file_t* files_buffer;

	//maybe need a specific structure
	unsigned int* f_blocks_buffer;
	unsigned long long* f_chunks_buffer;

	//not char* but message*, just test for message passing
	//char* m_cmd_buffer;
	msg_queue_t* m_cmd_queue;
	char* m_data_buffer;

	pthread_t* t_buffer;

	//信号量
	//signal for the m_cmd queue
	//...many other types of signal
};

typedef struct data_server data_server_t;

//define of some function in struct's operations
void init_data_sterver();
void init_lists();
void check_limits();//查看是否还允许分配
int get_current_imformation(struct data_server* server_imf);//返回目前数据节点的信息
void adjust_lists();
#endif
