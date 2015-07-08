/*
 * created on 2015.5.18
 * author: Binyang
 * used to accomplish synchronized visit
 */
#ifndef _SYN_TOOL_H_
#define _SYN_TOOL_H_
#include <stddef.h>
#include <pthread.h>
#include "basic_queue.h"

struct syn_queue
{
	basic_queue_t *queue;
	pthread_mutex_t *queue_mutex;
	pthread_cond_t *no_empty_cond;
	pthread_cond_t *no_full_cond;
};

typedef struct syn_queue syn_queue_t;

syn_queue_t* alloc_syn_queue(size_t size, int type_size);
void destroy_syn_queue(syn_queue_t *);
void syn_queue_push(syn_queue_t*, void*);
void syn_queue_pop(syn_queue_t*, void*);


#endif
