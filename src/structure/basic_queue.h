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
	void (*push)(struct basic_queue* , void* );
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
	void* (*dup)(void *ptr);
}basic_queue_t;


//you also can use pop and push function in message queue
basic_queue_t* alloc_msg_queue(int, int);
void destroy_msg_queue(basic_queue_t* this);

#endif
