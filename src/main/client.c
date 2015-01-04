/*
 * client.c
 *
 *  Created on: 2015年1月4日
 *      Author: ron
 */

/**
 * 1. 监听来自客户的请求
 */
#include "client.h"

int queue_empty(){
	return queue_size == 0;
}

void in_queue(const struct request_queue *request){
	queue_size++;
	queue_tail->next_request = request;
	queue_tail=request;
}

struct request_queue* de_queue(){
	if(queue_empty())
		return 0;
	queue_size--;
	struct request_queue *head = queue_head;
	queue_head = queue_head->next_request;
	return head;
}

void init(){
	queue_head = (struct message_queue *)calloc(1, sizeof(struct message_queue));
	queue_size = 0;
	queue_head = calloc(1, sizeof(struct request_queue));
}


void client_server() {
	struct request_queue *request;
	int send_status = MPI_Send((const void*) request,
			sizeof(struct request_queue), MPI_BYTE, master->rank, 1,
			master->comm);
}

