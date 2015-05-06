/*
 * variable_queue.c
 *
 *  Created on: 2015年5月6日
 *      Author: ron
 */
#include <stdio.h>
#include "variable_queue.h"

static double load_factor = 0.75;
static int default_capacity = 64;

variable_queue* variable_queue_create(){
	variable_queue *vq = (struct variable_queue*)malloc(sizeof(struct variable_queue));
	vq->queue = (struct variable_queue**)malloc(sizeof(struct variable_queue) * default_capacity);
	if(vq == NULL){
		return NULL;
	}
	vq->length = default_capacity;
	vq->head = 0;
	vq->tail = 0;
	vq->free = NULL;
	return vq;
}

static int variable_queue_resize(variable_queue* queue){
	int size = 1;
	while(size < queue->length){
		size = size << 1;
	}
	size = size << 1;

}

void variable_queue_free(variable_queue queue);
