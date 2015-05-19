/**
 * created on 2015.5.18
 * author: Binyang
 */
#include <stdlib.h>
#include <pthread.h>
#include "syn_tool.h"
#include "../structure/basic_queue.h"

queue_syn_t* alloc_queue_syn(void)
{
	int flag = 0;
	queue_syn_t* queue_syn;

	if((queue_syn = (queue_syn_t*)malloc(sizeof(queue_syn_t))) == NULL)
		return NULL;
	if((queue_syn->queue_mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t))) == NULL)
	{
		free(queue_syn);
		return NULL;
	}
	if((queue_syn->no_empty_cond = (pthread_cond_t*)malloc(sizeof(pthread_cond_t))) == NULL)
	{
		free(queue_syn->queue_mutex);
		free(queue_syn);
		return NULL;
	}
	if((queue_syn->no_full_cond = (pthread_cond_t*)malloc(sizeof(pthread_cond_t))) == NULL)
	{
		free(queue_syn->no_empty_cond);
		free(queue_syn->queue_mutex);
		free(queue_syn);
		return NULL;
	}

	if(pthread_mutex_init(queue_syn->queue_mutex, NULL))
		flag = 1;
	if(!flag && pthread_cond_init(queue_syn->no_empty_cond, NULL))
	{
		flag = 1;
		pthread_mutex_destroy(queue_syn->queue_mutex);
	}
	if(!flag && pthread_cond_init(queue_syn->no_full_cond, NULL))
	{
		flag = 1;
		pthread_mutex_destroy(queue_syn->queue_mutex);
		pthread_cond_destroy(queue_syn->no_empty_cond);
	}
	if(flag)
	{
		free(queue_syn->no_full_cond);
		free(queue_syn->no_empty_cond);
		free(queue_syn->queue_mutex);
		free(queue_syn);
		return NULL;
	}

	return queue_syn;
}

void destroy_queue_syn(queue_syn_t* queue_syn)
{
	pthread_cond_destroy(queue_syn->no_full_cond);
	pthread_cond_destroy(queue_syn->no_empty_cond);
	pthread_mutex_destroy(queue_syn->queue_mutex);

	free(queue_syn->no_full_cond);
	free(queue_syn->no_empty_cond);
	free(queue_syn->queue_mutex);
	free(queue_syn);
}

void syn_queue_push(basic_queue_t* queue, queue_syn_t* queue_syn, void* element)
{
	pthread_mutex_lock(queue_syn->queue_mutex);
	if(queue->basic_queue_op->is_full(queue))
	{
		pthread_cond_wait(queue_syn->no_full_cond, queue_syn->queue_mutex);
	}
	queue->basic_queue_op->push(queue, element);
	pthread_cond_signal(queue_syn->no_empty_cond);
	pthread_mutex_unlock(queue_syn->queue_mutex);
}

void syn_queue_pop(basic_queue_t* queue, queue_syn_t* queue_syn, void* element)
{
	pthread_mutex_lock(queue_syn->queue_mutex);
	if(queue->basic_queue_op->is_empty(queue))
	{
		pthread_cond_wait(queue_syn->no_empty_cond, queue_syn->queue_mutex);
	}
	queue->basic_queue_op->pop(queue, element);
	pthread_cond_signal(queue_syn->no_full_cond);
	pthread_mutex_unlock(queue_syn->queue_mutex);
}
