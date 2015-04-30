/**
 * created on 2015.4.28
 * author: Binyang
 * This is a long file
 */
#include <string.h>
#include <stdlib.h>
#include "threadpool.h"
#include "errinfo.h"

//message queue operation functions
void msg_queue_push(msg_queue_t* this, common_msg_t* msg )
{
	int offset;

	sem_wait(this->msg_queue_empty);
	offset = this->tail_pos;
	memcpy(this->msg + offset, msg, sizeof(common_msg_t));
	this->tail_pos = this->tail_pos + 1;
	sem_post(this->msg_queue_full);
}

void msg_queue_pop(msg_queue_t* this, common_msg_t* msg)
{
	int offset;

	sem_wait(this->msg_queue_full);
	offset = this->head_pos;
	memcpy(msg, this->msg + offset, sizeof(common_msg_t));
	this->head_pos = this->head_pos + 1;
	sem_post(this->msg_queue_empty);
}

msg_queue_t* alloc_msg_queue()
{
	msg_queue_t* this;
	this = (msg_queue_t* )malloc(sizeof(msg_queue_t));
	if(this == NULL)
	{
		err_ret("error in alloc_msg_queue");
		return NULL;
	}

	this->head_pos = 0;
	this->tail_pos = 0;
	//this->current_size = 0;
	this->msg = (common_msg_t* )malloc(sizeof(common_msg_t) * D_MSG_BSIZE);
	if(this->msg == NULL)
	{
		free(this);
		return NULL;
	}
	this->msg_queue_empty = (sem_t* )malloc(sizeof(sem_t));
	if(this->msg_queue_empty == NULL)
	{
		free(this->msg);
		free(this);
		return NULL;
	}
	this->msg_queue_full = (sem_t* )malloc(sizeof(sem_t));
	if(this->msg_queue_full == NULL)
	{
		free(this->msg_queue_empty);
		free(this->msg);
		free(this);
		return NULL;
	}
	sem_init(this->msg_queue_empty, 0, D_MSG_BSIZE);
	sem_init(this->msg_queue_full, 0, 0);

	this->msg_op = (msg_queue_op_t* )malloc(sizeof(msg_queue_op_t));
	if(this->msg_op == NULL)
	{
		free(this->msg_queue_full);
		free(this->msg_queue_empty);
		free(this->msg);
		free(this);
		return NULL;
	}
	this->msg_op->push = msg_queue_push;
	this->msg_op->pop = msg_queue_pop;
	return this;
}

void destroy_msg_queue(msg_queue_t* this)
{
	sem_destroy(this->msg_queue_empty);
	sem_destroy(this->msg_queue_full);
	free(this->msg_op);
	free(this->msg_queue_empty);
	free(this->msg_queue_full);
	free(this->msg);
	free(this);
}

/*================================EVENT HANDLER=======================================*/



/*====================================THREAD==========================================*/
static void* thread_do(void* arg)
{
	thread_t* this = (thread_t* )arg;
	int leader_id;
	pthread_mutex_lock(this->thread_pool->pool_mutex);
	for(;;)
	{
		leader_id = this->thread_pool->leader_id;
		if(leader_id != NO_THREAD_LEADER && this->id != leader_id)
		{
			pthread_cond_wait(this->thread_pool->pool_condition + this->id,
					this->thread_pool->pool_mutex);
		}

		leader_id = this->thread_pool->leader_id;
		pthread_mutex_unlock(this->thread_pool->pool_mutex);

		//get event from msg_queue
		//handle the event
		this->handler;

		//reenter monitor to serialize the program
		pthread_mutex_lock(this->thread_pool->pool_mutex);
	}
	return NULL;
}

static void init_thread(thread_t* thread, int id, event_handler_t* event_handler)
{
	thread->id = id;
	//This functions need to be modified
	//thread->handler = event_handler->handle_event;
	pthread_create(&(thread->pthread), NULL, thread_do, thread);
	pthread_detach(thread->pthread);
}

/*====================================THREAD POOL====================================*/

static pthread_cond_t* alloc_thread_cond(int threads_count)
{
	int i, j, err;

	pthread_cond_t* pthread_cond_arr;
	pthread_cond_arr = (pthread_cond_t* )malloc(sizeof(pthread_cond_t)* threads_count);
	if(pthread_cond_arr == NULL)
	{
		err_ret("error when allocate thread conditions");
		return NULL;
	}

	for(i = 0; i < threads_count; i++)
	{
		err = pthread_cond_init(&(pthread_cond_arr[i]), NULL);
		if(err != 0)
		{
			for(j = 0; j < i; j++)
				pthread_cond_destroy(&(pthread_cond_arr[j]));
			free(pthread_cond_arr);
			err_ret("error when allocate thread conditions");
			return NULL;
		}
	}
	return pthread_cond_arr;
}

thread_pool_t* alloc_thread_pool(int threads_count, msg_queue_t* msg_queue)
{
	thread_pool_t *thread_pool;
	thread_pool = (thread_pool_t*)malloc(sizeof(thread_pool_t));
	if(thread_pool == NULL)
	{
		err_ret("error in allocate thread_pool");
		return NULL;
	}

	thread_pool->threads = (thread_t* )malloc(sizeof(thread_t) * threads_count);
	thread_pool->spare_stack = (thread_t* )malloc(sizeof(thread_t) * threads_count);
	if(thread_pool->threads == NULL)
	{
		free(thread_pool);
		err_ret("error in allocate thread_pool");
		return NULL;
	}
	if(thread_pool->spare_stack == NULL)
	{
		free(thread_pool->threads);
		free(thread_pool);
		err_ret("error in allocate thread_pool");
		return NULL;
	}

	thread_pool->tp_ops = (threadpool_op_t* )malloc(sizeof(threadpool_op_t));
	if(thread_pool->tp_ops == NULL)
	{
		free(thread_pool->spare_stack);
		free(thread_pool->threads);
		free(thread_pool);
		err_ret("error in allocate thread_pool");
		return NULL;
	}

	thread_pool->pool_mutex = (pthread_mutex_t* )malloc(sizeof(pthread_mutex_t));
	if(pthread_mutex_init(thread_pool->pool_mutex, NULL) != 0)
	{
		free(thread_pool->pool_mutex);
		thread_pool->pool_mutex = NULL;
	}
	if(thread_pool->pool_mutex == NULL)
	{
		free(thread_pool->tp_ops);
		free(thread_pool->spare_stack);
		free(thread_pool->threads);
		free(thread_pool);
		err_ret("error in allocate thread_pool");
		return NULL;
	}

	thread_pool->pool_condition = alloc_thread_cond(threads_count);
	if(thread_pool->pool_condition == NULL)
	{
		pthread_mutex_destroy(thread_pool->pool_mutex);
		free(thread_pool->pool_mutex);
		free(thread_pool->tp_ops);
		free(thread_pool->spare_stack);
		free(thread_pool->threads);
		free(thread_pool);
		err_ret("error in allocate thread_pool");
		return NULL;
	}

	thread_pool->msg_queue = msg_queue;
	thread_pool->spare_stack_top = 0;
	thread_pool->threads_count = threads_count;
	thread_pool->pool_status = THREAD_POOL_UNINIT;
	thread_pool->leader_id = NO_THREAD_LEADER;
	return thread_pool;
}

static int push_stack(thread_pool_t* this, thread_t thread)
{
	thread_t* spare_stack = this->spare_stack;
	int top = this->spare_stack_top;
	if(top == this->threads_count)
		return -1;
	spare_stack[top] = thread;
	this->spare_stack_top = this->spare_stack_top + 1;
	return 0;
}

static int pop_stack(thread_pool_t* this, thread_t* stack_top)
{
	thread_t* spare_stack = this->spare_stack;
	int top = this->spare_stack_top;
	if(top == 0)
	{
		err_msg("wrong threads spare stack");
		return -1;
	}
	*stack_top = spare_stack[top - 1];
	this->spare_stack_top = this->spare_stack_top - 1;
	return 0;
}

void start(thread_pool_t* this, event_handler_t* event_handler)
{
	int i;
	if(this->pool_status == THREAD_POOL_UNINIT)
	{
		//the first thread will be the leader
		pthread_mutex_lock(this->pool_mutex);
		this->leader_id = 0;
		pthread_mutexunlock(this->pool_mutex);

		for(i = 1; i < this->threads_count; i++)
		{
			init_thread(&(this->threads[i]), i, event_handler);
			push_stack(this, this->threads[i]);
		}
		this->pool_status = THREAD_POOL_INIT;
	}
	return;
}
