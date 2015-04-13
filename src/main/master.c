/*
 * master.c
 *
 *  Created on: 2015年1月4日
 *      Author: ron
 */
#include "master.h"
#include <assert.h>

void init_queue() {
	request_queue_list.head = request_queue_list.tail = NULL;
	request_queue_list.request_num = 0;
}

int request_is_empty() {
	return !request_queue_list.request_num;
}

void init_master_server_info(int machine_count){
	master_data_servers.server_count = machine_count;
	master_data_servers.data_server_list = (data_server_des*)malloc(sizeof(data_server_des) * machine_count);
	assert(master_data_servers.data_server_list != NULL);
}

void in_queue(request_node *request_param) {
	log_info("====start in queue=====");
	if (request_is_empty()) {
		request_queue_list.tail = request_queue_list.head = request_param;
	} else {
		request_queue_list.tail->next = request_param;
		request_queue_list.tail = request_param;
	}
	request_queue_list.request_num++;
	log_info("====end in queue=====");
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
	tmp_request->next = 0;
	mpi_status_assignment(&(tmp_request->status), status);
	return tmp_request;
}

void free_request_node(request_node *request) {
	free(request->message);
	free(request);
}

/**
 *
 */
void data_server_init() {
	init_queue();
	//master_data_servers.index = 1;
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
	//puts("=====master initialize start=====");
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
	//puts("=====master initialize end=====");
}

/**
 *	master server
 */
void* master_server(void *arg) {
	log_info("master server start");
	MPI_Status status;
	init_queue();
	while (1) {
		pthread_mutex_lock(&mutex_message_buff);
		log_info("receive message start");
		MPI_Recv(message_buff, MAX_CMD_MSG_LEN, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		log_info("receive message end");
		//TODO...
		//need to catch status error

		request_node *reqeust = malloc_request(message_buff, MAX_CMD_MSG_LEN, &status);
		pthread_mutex_lock(&mutex_request_queue);
		in_queue(reqeust);
		log_info("signal the thread waiting request queue start");
		pthread_cond_signal(&cond_request_queue);
		pthread_mutex_unlock(&mutex_request_queue);
		pthread_mutex_unlock(&mutex_message_buff);
		log_info("signal the thread waiting request queue end");
	}
	pthread_cond_destroy(&cond_request_queue);
	return 0;
}

file_location_des *maclloc_data_block(unsigned long file_size) {
	file_location_des * t = (file_location_des*) malloc(sizeof(file_location_des));
	unsigned long block_count = ceil((double)file_size / BLOCK_SIZE);
	t->machinde_count = 1;
	return t;
}

void answer_client_create_file(request_node *request){
	create_file_structure *create = request->message;
	file_location_des *file_location = maclloc_data_block(create->file_size);
	int i = 0;
	for(; i != file_location->machinde_count; i++){

	}
	strcpy(send_message_buff, "hello world!!");
	MPI_Status *status = &(request->status);
	MPI_Send(send_message_buff, MAX_CMD_MSG_LEN, MPI_CHAR,
			status->MPI_SOURCE, CLIENT_INSTRUCTION_ANS_MESSAGE,
			MPI_COMM_WORLD);
	free_request_node(request);
}

void* request_handler(void *arg) {
	request_node *request;
	while (1) {
		pthread_mutex_lock(&mutex_request_queue);
		log_info("dequeue loop start");
		if (request_is_empty())
			pthread_cond_wait(&cond_request_queue, &mutex_request_queue);
		request = de_queue();
		log_info("dequeue loop end");
		pthread_mutex_unlock(&mutex_request_queue);
		if (request != NULL) {
			unsigned short *operation_code = request->message;
			if (*operation_code == CREATE_FILE_CODE) {
				answer_client_create_file(request);
			}
		}
	}
	return 0;
}
