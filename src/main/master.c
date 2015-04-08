/*
 * master.c
 *
 *  Created on: 2015年1月4日
 *      Author: ron
 */
#include "master.h"

void init_queue() {
	request_queue_list.head = request_queue_list.tail = NULL;
	request_queue_list.request_num = 0;
}

int request_is_empty() {
	return !request_queue_list.request_num;
}

void in_queue(request_node *request_param) {
	puts("===in queue start==");
	request_queue_list.tail->next = request_param;
	request_queue_list.tail = request_param;
	request_queue_list.request_num++;
}

request_node* de_queue() {
	if (request_is_empty())
		return NULL;
	request_node *tmp_request = request_queue_list.head;
	request_queue_list.head = tmp_request->next;
	request_queue_list.request_num--;
	return tmp_request;
}

void mpi_status_assignment(MPI_Status *status, MPI_Status *s) {
	memcpy(status, s, sizeof(MPI_Status));
}

request_node* malloc_request(char *buf, int size, MPI_Status *status) {
	request_node *tmp_request = (request_node*) malloc(sizeof(request_node));
	tmp_request->message = (char *) malloc(size);
	strncpy(tmp_request->message, buf, size);
	mpi_status_assignment(&(tmp_request->status), status);
	return tmp_request;
}

void free_request_node(request_node *request) {
	free(request->message);
	free(request);
}

void data_server_init() {
	master_data_servers.index = 1;
	master_data_servers.server_count = 0;
	master_data_servers.data_server_list = (data_server_des *) malloc(
			sizeof(data_server_des) * master_data_servers.server_count);
	memset(&master_data_servers, 0,
			sizeof(data_server_des) * master_data_servers.server_count);
}

/**
 * 1. name space lock initialize
 * 2. message buffer lock initialize
 * 3. request handler start
 * 4. master server start
 */
void master_init() {
	puts("=====master initialize start=====");
//	MPI_Status status;
//	MPI_Recv(message_buff, MAX_COM_MSG_LEN, MPI_CHAR, MPI_ANY_SOURCE,
//	MPI_ANY_TAG, MPI_COMM_WORLD, &status);
//	puts(message_buff);
	pthread_mutex_init(&mutex_message_buff, NULL);
	pthread_mutex_init(&mutex_namespace, NULL);

	pthread_create(&thread_master_server, NULL, master_server, NULL);
	pthread_create(&thread_request_handler, NULL, request_handler, NULL);

	pthread_join(thread_master_server, NULL);
	pthread_join(thread_request_handler, NULL);

	pthread_cond_init(&cond_request_queue, NULL);
	puts("=====master initialize end=====");
}

/**
 *	master的服务
 */
void* master_server(void *arg) {
	puts("master server start");
	MPI_Status status;
	init_queue();
	while (1) {
		pthread_mutex_lock(&mutex_message_buff);
		puts("receive message start");
		MPI_Recv(message_buff, MAX_COM_MSG_LEN, MPI_CHAR, MPI_ANY_SOURCE,
		MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		puts("receive message end");
		//TODO...
		//need to catch status error

		request_node *reqeust = malloc_request(message_buff, MAX_COM_MSG_LEN, &status);
		pthread_mutex_lock(&mutex_request_queue);
		printf("information = %d\n", request_queue_list.request_num);
		in_queue(reqeust);
		printf("after in_queue information = %d\n", request_queue_list.request_num);
		pthread_cond_signal(&cond_request_queue);
		printf("information = %d\n", request_queue_list.request_num);
		pthread_mutex_unlock(&mutex_request_queue);
		pthread_mutex_unlock(&mutex_message_buff);
	}
	pthread_cond_destroy(&cond_request_queue);
	return 0;
}

file_location_des *maclloc_data_block(unsigned long file_size) {
	file_location_des * t = (file_location_des*) malloc(
			sizeof(file_location_des));
	t->machinde_count = 1;
	return t;
}

void* request_handler(void *arg) {
	request_node *request;
	while (1) {
		pthread_mutex_lock(&mutex_request_queue);
		puts("deque loop start");
		pthread_cond_wait(&cond_request_queue, &mutex_request_queue);
		//request = de_queue();
		puts("deque loop");
		pthread_mutex_unlock(&mutex_request_queue);
//		if (request != NULL) {
//			unsigned short *operation_code = request->message;
//			if (*operation_code == CREATE_FILE_CODE) {
//				create_file_structure *create = request->message;
//				strcpy(send_message_buff, "hello world!!");
//				MPI_Status *status = &(request->status);
//				MPI_Send(send_message_buff, MAX_COM_MSG_LEN, MPI_CHAR,
//						status->MPI_SOURCE, status->MPI_TAG, MPI_COMM_WORLD);
//				free_request_node(request);
//			}
//		}
	}
	return 0;
}

/**
 * master记录日志
 */
void log_backup() {

}

/*
 * master接受心跳信息
 */
void heart_blood() {

}

/**
 *	命名空间的操作
 */
void namespace_control() {
	while (1) {

	}
}

