/**
 * created on 2015.4.16
 * author: Binyang
 * This file will offer many functions to handle the data server's
 * buffer. dataserver.h must include this file
 */
#ifndef _DATASERVER_BUFF_H_
#define _DATASERVER_BUFF_H_

#include <stdio.h>
#include <semaphore.h>
#include "../../tool/message.h"
#include "../../global.h"

//empty head_pos == tail_pos; full (tail_pos + 1) % size == head_pos
#define Q_FULL(head_pos, tail_pos) (((tail_pos + 1) % D_MSG_BSIZE) == (head_pos))
#define Q_EMPTY(head_pos, tail_pos) ((head_pos) == (tail_pos))

struct msg_queue;

struct msg_queue_op
{
	void (*push)(struct msg_queue* , common_msg_t* );
	common_msg_t* (*pop)(struct msg_queue* );
};

struct msg_queue
{
	//int current_size;
	int head_pos;
	int tail_pos;
	sem_t* msg_queue_full;
	sem_t* msg_queue_empty;
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

typedef struct msg_queue msg_queue_t;

//followings are operation functions
void msg_queue_push(msg_queue_t*, common_msg_t* );
void msg_queue_pop(msg_queue_t* , common_msg_t* );

//init function
void alloc_msg_queue(msg_queue_t* );
void destroy_msg_queue(msg_queue_t* );

#endif
//void
