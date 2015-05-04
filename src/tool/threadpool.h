/**
 * created on: 2015.4.28
 * author: Binyang
 * This file want to create a thread pool used by data server and may other area.
 * We used leader and follower model to create it and this file will also define some
 * data structure about message queue and other buffer operations.
 * Here we go!!!
 */
#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include "message.h"
#include "../global.h"

#define THREAD_POOL_UNINIT 0
#define THREAD_POOL_INIT 1

#define NO_THREAD_LEADER -1
//empty head_pos == tail_pos; full (tail_pos + 1) % size == head_pos
#define Q_FULL(head_pos, tail_pos) (((tail_pos + 1) % D_MSG_BSIZE) == (head_pos))
#define Q_EMPTY(head_pos, tail_pos) ((head_pos) == (tail_pos))

struct msg_queue;
struct thread_pool;

struct msg_queue_op
{
	void (*push)(struct msg_queue* , common_msg_t* );
	void (*pop)(struct msg_queue* , common_msg_t* );
};

/*
 * this msg_queue do not provide mutex lock
 * if multiple threads will modify it, you should
 * lock the mutex by yourself
 */
struct msg_queue
{
	//int current_size;
	int head_pos;
	int tail_pos;
	struct msg_queue_op* msg_op;
	sem_t* msg_queue_full;
	sem_t* msg_queue_empty;
	pthread_mutex_t* msg_mutex;
	common_msg_t* msg;
};


struct buffer
{
	char *buffer;
	struct buffer* next;
};

//The event_handler should run by leader thread, there should be a common
//event handler and different layer use different formats
struct event_handler
{
	void (*handler)(struct event_handler*);
	void* (*resolve_handler)(struct event_handler*, void* args);
	void* spcical_struct;

	struct buffer* event_buffer;
	struct thread_pool* thread_pool;
};

struct thread
{
	int id;
	struct thread_pool* thread_pool;
	struct event_handler* event_handler;
	pthread_t pthread;
	int (*handler)(struct thread*, struct event_handler*);
};

struct threadpool_opertions
{
	int (*promote_a_leader)(struct thread_pool*, struct thread*);
	//every thread run join function at the beginning
	void (*start)(struct thread_pool*, struct event_handler*);
	void (*deactive_handle)(struct thread_pool*);
	void (*reactive_handle)(struct thread_pool*);
};

struct thread_pool
{
	struct thread* threads;
	struct thread* spare_stack;

	pthread_cond_t* pool_condition;
	pthread_mutex_t* pool_mutex;

	int leader_id;
	int spare_stack_top;
	int threads_count;
	int pool_status;

	struct threadpool_opertions* tp_ops;
	struct msg_queue* msg_queue;
};

/*================================typedef=========================================*/
typedef struct msg_queue msg_queue_t;
typedef struct msg_queue_op msg_queue_op_t;

typedef struct thread_pool thread_pool_t;
typedef struct threadpool_opertions threadpool_op_t;
typedef struct thread thread_t;

typedef struct event_handler event_handler_t;
typedef void (*handler_t)(event_handler_t*);
typedef void* (*resolve_handler_t)(event_handler_t*, void* args);

/*===============================message queue===================================*/

//followings are operation functions
void msg_queue_push(msg_queue_t*, common_msg_t* );
void msg_queue_pop(msg_queue_t* , common_msg_t* );

//init function
msg_queue_t* alloc_msg_queue();
void destroy_msg_queue(msg_queue_t* );

/*===============================thread pool=======================================*/

thread_pool_t* alloc_thread_pool(int threads_count, msg_queue_t*);
void distroy_thread_pool(thread_pool_t*);
void start(thread_pool_t*, event_handler_t*);

/*===============================event handler=====================================*/
event_handler_t* alloc_event_handler(thread_pool_t*, resolve_handler_t);
void destory_event_handler(event_handler_t*);
#endif
