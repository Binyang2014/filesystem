/*
 * message_queue.h
 *
 *  Created on: 2015年5月5日
 *      Author: ron
 */

#ifndef SRC_MAIN_MESSAGE_QUEUE_H_
#define SRC_MAIN_MESSAGE_QUEUE_H_

typedef struct queue_node{
	struct queue_node *pre;
	struct queue_node *next;
	void *message;
}queue_node;

typedef struct queue{
	struct queue_node *head;
	struct queue_node *tail;
	unsigned int len;
	void (*free)(void *ptr);
}queue;



/* Functions implemented as macros */
#define queue_length(q) ((q)->len)
#define queue_first(q) ((q)->head)
#define queue_last(q) ((q)->tail)
#define queue_prev_node(n) ((n)->prev)
#define queue_next_node(n) ((n)->next)
#define queue_node_value(n) ((n)->value)

//#define listSetDupMethod(l,m) ((l)->dup = (m))
#define queue_set_free_method(q, m) ((q)->free = (m))
//#define listSetMatchMethod(l,m) ((l)->match = (m))

//#define listGetDupMethod(l) ((l)->dup)
#define queue_get_free(q) ((q)->free)
//#define listGetMatchMethod(l) ((l)->match)

/* Prototypes */
queue *queue_create(void);
void queue_release(queue *queue);
//list *queue_add_node_head(queue *queue, void *message);
queue *queue_add_node_tail(queue queue, void *message);

queue_node *queue_pop(queue *queue);
//list *queue_insert_node(queue *queue, queue_node *old_node, void *message, int after);
//void listDelNode(list *list, listNode *node);
//listIter *listGetIterator(list *list, int direction);
//listNode *listNext(listIter *iter);
//void listReleaseIterator(listIter *iter);
//list *listDup(list *orig);
//listNode *listSearchKey(list *list, void *key);
//listNode *listIndex(list *list, long index);
//void listRewind(list *list, listIter *li);
//void listRewindTail(list *list, listIter *li);
//void listRotate(list *list);

#endif /* SRC_MAIN_MESSAGE_QUEUE_H_ */
