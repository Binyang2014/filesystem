/*
 * variable_queue.h
 *
 *  Created on: 2015年5月6日
 *      Author: ron
 */

#ifndef SRC_STRUCTURE_VARIABLE_QUEUE_H_
#define SRC_STRUCTURE_VARIABLE_QUEUE_H_

typedef struct variable_queue_node{
	struct variable_queue_node *prev;
	struct variable_queue_node *next;
	void *value;
}variable_queue_node;

typedef struct variable_queue{
	struct variable_queue_node **queue;
	void (*free)(void *ptr);
	unsigned int length;
	unsigned int head;
	unsigned int tail;
}variable_queue;

#define variable_queue_len(q) ((q)->length)

variable_queue* variable_queue_create();
//int variable_queue_resize(unsigned int size);
void variable_queue_free(variable_queue queue);



#endif /* SRC_STRUCTURE_VARIABLE_QUEUE_H_ */
