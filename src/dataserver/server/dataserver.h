/*
 * datasever.h
 *
 * Created on 2015.1.23
 * Author:binyang
 */
#ifndef _FILESYSTEM_DATASERVER_H_
#define _FILESYSTEM_DATASERVER_H_

#define Q_FULL(head_pos, tail_pos) (((tail_pos + 1) % D_MSG_BSIZE) == (head_pos))
#define Q_EMPTY(head_pos, tail_pos) ((head_pos) == (tail_pos))

#include <pthread.h>
#include "dataserver_comm_handler.h"
#include "../structure/vfs_structure.h"
#include "../../tool/message.h"

//empty head_pos == tail_pos; full (tail_pos + 1) % size == head_pos
struct msg_queue
{
	int current_size;
	int head_pos;
	int tail_pos;
	common_msg_t* msg;
};

//tread thread pool as a queue??
struct thread_pool
{
	pthread_t* threads;
	int current_size;
	int head_pos;
	int tail_pos;
};

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
	vfs_hashtable_t* f_arr_buff;

	//not char* but message*, just test for message passing
	//char* m_cmd_buffer;
	struct msg_queue* m_cmd_queue;
	char* m_data_buffer;//associate with number of thread??

	struct thread_pool* t_buffer;
};

typedef struct data_server data_server_t;
typedef struct msg_queue msg_queue_t;

//define of some function in struct's operations
//about message receive and resolve
void* m_cmd_receive(void * msg_queue_arg);//there will be a thread run this function
void m_resolve();

//about data server
data_server_t* init_dataserver(total_size_t t_size, int dev_num);
void init_lists();
void check_limits();//查看是否还允许分配
int get_current_imformation(struct data_server* server_imf);//返回目前数据节点的信息

//this is the ultimate goal!
void datasecer_run();
#endif
