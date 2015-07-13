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

struct syn_queue;

struct syn_queue_op {
	void (*syn_queue_push)(struct syn_queue *syn_queue, void *element);
	void (*syn_queue_pop)(struct syn_queue *syn_queue, void *element);
};

struct syn_queue
{
	basic_queue_t *queue;
	pthread_mutex_t *queue_mutex;
	pthread_cond_t *no_empty_cond;
	pthread_cond_t *no_full_cond;
	struct syn_queue_op *op;
};

typedef struct syn_queue syn_queue_t;
typedef struct syn_queue_op syn_queue_op_t;

syn_queue_t* alloc_syn_queue(size_t size, int type_size);
void destroy_syn_queue(syn_queue_t *);

#endif
