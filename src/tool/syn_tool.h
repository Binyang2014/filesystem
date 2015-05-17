/*
 * created on 2015.5.18
 * author: Binyang
 * used to accomplish synchronized visit
 */
#ifndef _SYN_TOOL_H_
#define _SYN_TOOL_H_
#include <pthread.h>
#include "../structure/basic_queue.h"

struct queue_syn
{
	pthread_mutex_t *queue_mutex;
	pthread_cond_t* no_empty_cond;
	pthread_cond_t* no_full_cond;
};

typedef struct queue_syn queue_syn_t;

queue_syn_t* alloc_queue_syn(void);
void destroy_queue_syn(queue_syn_t*);

static inline int lock_the_queue(queue_syn_t* queue_syn)
{
	return pthread_mutex_lock(queue_syn->queue_mutex);
}

static inline int unlock_the_queue(queue_syn_t* queue_syn)
{
	return pthread_mutex_unlock(queue_syn->queue_mutex);
}

void syn_queue_push(basic_queue_t*, queue_syn_t*, void*);
void syn_queue_pop(basic_queue_t*,  queue_syn_t*, void*);

#endif
