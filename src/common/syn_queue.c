/**
 * created on 2015.5.18
 * author: Binyang
 */
#include <stdlib.h>
#include <pthread.h>

#include "syn_queue.h"
#include "zmalloc.h"

syn_queue_t* alloc_syn_queue(size_t size, int type_size)
{
	syn_queue_t* syn_queue = (syn_queue_t*)zmalloc(sizeof(syn_queue_t));
	syn_queue->queue = alloc_basic_queue(size, type_size);
	syn_queue->queue_mutex = (pthread_mutex_t*)zmalloc(sizeof(pthread_mutex_t));
	syn_queue->no_empty_cond = (pthread_cond_t*)zmalloc(sizeof(pthread_cond_t));
	syn_queue->no_full_cond = (pthread_cond_t*)zmalloc(sizeof(pthread_cond_t));
	pthread_mutex_init(syn_queue->queue_mutex, NULL);
	pthread_cond_init(syn_queue->no_empty_cond, NULL);
	pthread_cond_init(syn_queue->no_full_cond, NULL);

	return syn_queue;
}

void destroy_syn_queue(syn_queue_t* syn_queue)
{
	if(syn_queue == NULL){
		return;
	}

	pthread_cond_destroy(syn_queue->no_full_cond);
	pthread_cond_destroy(syn_queue->no_empty_cond);
	pthread_mutex_destroy(syn_queue->queue_mutex);
	destroy_basic_queue(syn_queue->queue);

	zfree(syn_queue->no_full_cond);
	zfree(syn_queue->no_empty_cond);
	zfree(syn_queue->queue_mutex);
	zfree(syn_queue);
}

void syn_queue_push(syn_queue_t* syn_queue, void* element)
{
	pthread_mutex_lock(syn_queue->queue_mutex);
	while(syn_queue->queue->basic_queue_op->is_full(syn_queue->queue))
	{
		pthread_cond_wait(syn_queue->no_full_cond, syn_queue->queue_mutex);
	}
	syn_queue->queue->basic_queue_op->push(syn_queue->queue, element);
	pthread_cond_signal(syn_queue->no_empty_cond);
	pthread_mutex_unlock(syn_queue->queue_mutex);
}

void syn_queue_pop(syn_queue_t* syn_queue, void* element)
{
	pthread_mutex_lock(syn_queue->queue_mutex);
	while(syn_queue->queue->basic_queue_op->is_empty(syn_queue->queue))
	{
		pthread_cond_wait(syn_queue->no_empty_cond, syn_queue->queue_mutex);
	}
	syn_queue->queue->basic_queue_op->pop(syn_queue->queue, element);
	pthread_cond_signal(syn_queue->no_full_cond);
	pthread_mutex_unlock(syn_queue->queue_mutex);
}

#define SYN_QUEUE_TEST 1
#if defined(GLOBAL_TEST) || defined(SYN_QUEUE_TEST)
int main() {
	return 0;
}
#endif
