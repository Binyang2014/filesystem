/**
 * dataserver_communicate.h
 *
 * created on 2015.1.25
 * author binyang
 *
 * this file define some function used to communicate with master and client
 * following functions and structures this file should define
 * 1. about message queue and message buffer
 * 2. about threads pool
 * 3. send and receive message from master and client
 * 4. cultivate right thread to do right things
 * 5. manipulate the buffers
 */
#ifndef _DATASERVER_COMM_H_
#define _DARASERVER_COMM_H_

#define Q_FULL(head_pos, tail_pos) (((tail_pos + 1) % D_MSG_BSIZE) == (head_pos))
#define Q_EMPTY(head_pos, tail_pos) ((head_pos) == (tail_pos))

#include "../../tool/message.h"
#include "../../global.h"
#include "../structure/vfs_structure.h"

//empty head_pos == tail_pos; full (tail_pos + 1) % size == head_pos
struct msg_queue
{
	int current_size;
	int head_pos;
	int tail_pos;
	common_msg_t* msg;
};

struct msg_for_rw
{
	dataserver_file_t* file;
	off_t offset;
	size_t count;
};

typedef struct msg_queue msg_queue_t;
typedef struct msg_for_rw msg_for_rw_t;

void init_msg();
void m_cmd_receive(msg_queue_t * msg_queue);//there is a thread run this function

int m_read_handler(int source, int tag, msg_for_rw_t* file_info, char* buff, void* msg_buff);
int m_write_handler(int source, int tag, msg_for_rw_t* file_info, char* buff,  void* msg_buff);
//...other message passing functions
#endif
