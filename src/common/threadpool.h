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
#include "basic_list.h"
#include "basic_queue.h"
#include "syn_queue.h"

#define THREAD_POOL_UNINIT 0
#define THREAD_POOL_INIT 1

#define NO_THREAD_LEADER -1

/*---------------------------------------------------------------------------------*/
struct thread;
struct thread_pool;
struct threadpool_opertions;

struct event_handler;
struct event_handler_set;

/*-----------------------------------------------------------------------------*/

//The event_handler should run by leader thread, there should be a common
//event handler and different layer use different formats
struct event_handler
{
	void (*handler)(struct event_handler*);
	void* (*resolve_handler)(struct event_handler*, void* args);
	void* spcical_struct;

	list_t* event_buffer_list;
	struct thread_pool* thread_pool;
};

struct event_handler_set
{
	int event_handlers_conut;
	struct event_handler** evnet_handler_arr;
};

/*-----------------------------------------------------------------------------*/

struct thread
{
	int id;
	char canceled;
	struct thread_pool* thread_pool;
	struct event_handler* event_handler;
	pthread_t pthread;
	int (*handler)(struct thread*, struct event_handler*);
};

struct threadpool_opertions
{
	int (*promote_a_leader)(struct thread_pool*, struct thread*);
	//every thread run join function at the beginning
	void (*start)(struct thread_pool*, struct event_handler_set*);
	void (*deactive_handle)(struct thread_pool*);
	void (*reactive_handle)(struct thread_pool*);
};

struct thread_pool
{
	struct thread* threads;
	struct thread* spare_stack;

	pthread_cond_t* pool_condition;
	pthread_mutex_t* pool_mutex;
	pthread_mutex_t* handle_mutex;

	int leader_id;
	int spare_stack_top;
	int threads_count;
	int pool_status;
	int spare_treads_count;

	struct threadpool_opertions* tp_ops;
	struct basic_queue* msg_queue;
};

/*================================typedef=========================================*/
typedef struct thread_pool thread_pool_t;
typedef struct threadpool_opertions threadpool_op_t;
typedef struct thread thread_t;

typedef struct event_handler event_handler_t;
typedef struct event_handler_set event_handler_set_t;
typedef void (*handler_t)(event_handler_t*);
typedef void* (*resolve_handler_t)(event_handler_t*, void* args);


/*===============================thread pool=======================================*/

//there are promote, deactive, reactive and start functions
thread_pool_t* alloc_thread_pool(int threads_count, basic_queue_t *);
void distroy_thread_pool(thread_pool_t *);

/*===============================event handler=====================================*/

//there are handler and resolve_handler functions you should concern
//resolve function will resolve message and return handle function for this event
//the handler will do the right things for the event
event_handler_set_t *alloc_event_handler_set(thread_pool_t*, resolve_handler_t);
void destroy_event_handler_set(event_handler_set_t*);

#endif
