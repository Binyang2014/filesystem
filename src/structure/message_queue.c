/*
 * message_queue.c
 *
 *  Created on: 2015年5月5日
 *      Author: ron
 */
#include <stdio.h>
#include "message_queue.h"

queue *queue_create(void){
	queue *queue = (struct queue*)malloc(sizeof (struct queue));
	queue->len = 0;
	queue->head = NULL;
	queue->tail = NULL;
	queue->free = NULL;
	return queue;
}

queue *queue_add_node_tail(queue queue, void *message){
	if(queue->tail){
		queue_node node = (queue_node*)malloc(sizeof(struct queue_node));
		if(node){
			node->message = message;
			node->next = NULL;
			node->pre = queue->tail;
			queue->tail->next = node;
			queue->tail = node;
			queue->len++;
		}
	}
	return queue;
}

queue_node *queue_pop(queue *queue){
	queue_node *node = queue->head;
	if(node){
		queue->head = queue->head->next;
		if(queue->head)
			queue->head->pre = NULL;
		node->pre = NULL;
		node->next = NULL;
	}
	return node;
}

