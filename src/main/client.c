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

}

int in_queue(){

}

int de_queue(){

}

void init(){
	queue = (struct message_queue *)calloc(1, sizeof(struct message_queue));
	queue->queue_size = 0;
	queue->
}


void client_server() {
	struct request_queue *request;
	int send_status = MPI_Send((const void*) request,
			sizeof(struct request_queue), MPI_BYTE, master->rank, 1,
			master->comm);
}

