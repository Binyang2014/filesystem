/*
 * This file provide a ordinary queue, this file havn't been tested yet.
 */
#ifndef _BASIC_QUEUE_H_
#define _BASIC_QUEUE_H_

#define queue_set_dup_method(q,m) ((q)->dup = (m))
#define queue_set_free_method(q,m) ((q)->free = (m))

struct basic_queue;
struct basic_queue_op;

typedef struct basic_queue_op
{
	int (*push)(struct basic_queue* , void* );
	void (*pop)(struct basic_queue* , void* );
	int  (*is_empty)(struct basic_queue*);
	int  (*is_full)(struct basic_queue*);
}basic_queue_op_t;

/*
 * this basic_queue do not provide mutex lock
 * if multiple threads will modify it, you should
 * lock the mutex by yourself
 */
typedef struct basic_queue
{
	int head_pos;
	int tail_pos;
	int current_size;
	int queue_len;
	int element_size;
	basic_queue_op_t* basic_queue_op;
	void* elements;
	void (*free)(void *ptr);
	void (*dup)(void *dest, void *source);
}basic_queue_t;

typedef struct basic_queue_iterator{
	basic_queue_t *queue;
	int offset;
	int it_size;
	int (*has_next)(struct basic_queue_iterator *);
	void (*next)(struct basic_queue_iterator *, void *dest);
}basic_queue_iterator;

#define queue_set_free(q, m) ((q)->free=(m))
#define queue_set_dup(q, m) ((q)->dup=(m))
//you also can use pop and push function in message queue

basic_queue_iterator *create_basic_queue_iterator(basic_queue_t *queue);

/*TODO, allocate file space can use this reuse old queue space */
basic_queue_t* alloc_basic_queue(int, int);
void destroy_basic_queue(basic_queue_t* this);

#endif
