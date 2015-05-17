/*
 * master.c
 *
 *  Created on: 2015年1月4日
 *      Author: ron
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <mpi.h>
#include <assert.h>
#include "master.h"
#include "./namespace/namespace.h"
#include "./data_server/master_data_servers.h"
#include "../../structure/basic_queue.h"
#include "../../tool/message.h"
#include "../global.h"
#include "conf.h"

/*====================Private Prototypes====================*/
static namespace* namespace;
static basic_queue_t* message_queue;
static data_servers *master_data_server;

static pthread_t *pthread_request_listener;
static pthread_t *pthread_request_handler;
static pthread_t *pthread_damon_handler;
static pthread_mutex_t *mutex_message_queue;
static pthread_cond_t *cond_message_queue;

static char *receive_buf;
static char *send_buf;
static common_msg_t *msg_buff;
static common_msg_t *msg_pop_buff;
static common_msg_t *request_handle_buff;

static void set_common_msg(common_msg_t msg, int source, char *message);
static void *master_server();

static int master_create_file(char *file_name, unsigned long file_size);

/*====================private functions====================*/
static void mpi_status_assignment(MPI_Status *status, MPI_Status *s) {
	memcpy(status, s, sizeof(MPI_Status));
}

static void msg_dup(void *dest, void *source){
	memcpy(dest, source, sizeof(common_msg_t));
}

static void msg_free(void *msg){
	free(msg);
}

static void set_common_msg(common_msg_t msg, int source, char *message){
	unsigned short *operation_code = message;
	msg->operation_code = *operation_code;
	msg->source = source;
	strncpy(msg->rest, message, MAX_CMD_MSG_LEN);
}

static int queue_push_msg(basic_queue_t *queue, int source, char *message){
	set_common_msg(msg_buff, source, receive_buf);
	queue->basic_queue_op->push(msg_buff);
}

/*====================API Implementation====================*/
//TODO when shutdown the application, it must release all the memory resource
int master_init(){
	namespace = create_namespace(1024, 32);
	message_queue = alloc_msg_queue(sizeof(common_msg_t), -1);
	master_data_server = data_servers_create(1024, 0.75, 10);
	queue_set_free(message_queue, msg_free);
	queue_set_dup(message_queue, msg_dup);

	pthread_request_listener = (pthread_t *)malloc(sizeof(pthread_t));
	pthread_request_handler = (pthread_t *)malloc(sizeof(pthread_t));
	mutex_message_queue = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
	cond_message_queue = (pthread_cond_t *)malloc(sizeof(cond_message_queue));

	receive_buf = (char *)malloc(MAX_CMD_MSG_LEN);
	send_buf = (char *)malloc(MAX_CMD_MSG_LEN);
	msg_buff = (common_msg_t *)malloc(sizeof(common_msg_t));
	msg_pop_buff = (common_msg_t *)malloc(sizeof(common_msg_t));
	request_handle_buff = (common_msg_t *)malloc(sizeof(common_msg_t));
	if(namespace == NULL || message_queue == NULL || pthread_request_listener == NULL
			|| pthread_request_handler == NULL || mutex_message_queue == NULL){
		master_destroy();
		return -1;
	}
}

/**
 *	master server
 *	接收消息，加入队列
 */
static void* master_server(void *arg) {
	MPI_Status status;
	while (1) {
		MPI_Recv(receive_buf, MAX_CMD_MSG_LEN, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		//TODO need to catch status error

		pthread_mutex_lock(&mutex_message_queue);
		queue_push_msg(message_queue, status.MPI_SOURCE, receive_buf);
		pthread_cond_signal(&cond_message_queue);
		pthread_mutex_unlock(&mutex_message_queue);
	}
	pthread_cond_destroy(&cond_message_queue);
	return 0;
}

static void* request_handler(void *arg) {
	while (1) {
		pthread_mutex_lock(&mutex_message_queue);
		log_info("dequeue loop start");
		while (message_queue->basic_queue_op->is_empty()){
			pthread_cond_wait(&cond_message_queue, &mutex_message_queue);
		}
		message_queue->basic_queue_op->pop(message_queue, msg_pop_buff);
		pthread_mutex_unlock(&mutex_message_queue);
		if (msg_pop_buff != NULL) {
			unsigned short *operation_code = msg_pop_buff->operation_code;
//			if (*operation_code == CREATE_FILE_CODE) {
//				answer_client_create_file(request);
//			}
		}
	}
	return 0;
}

//TODO dangerous operation, when shutdown the application
//TODO self-defined structure may be released by it's own free function
int master_destroy(){
	free(namespace);
	free(message_queue);
	free(pthread_request_listener);
	free(pthread_request_handler);
	free(mutex_message_queue);
}

/**
 * 1. name space lock initialize
 * 2. message buffer lock initialize
 * 3. request handler start
 * 4. master server start
 */
//void master_init() {
//	//puts("=====master initialize start=====");
////	MPI_Status status;
////	MPI_Recv(message_buff, MAX_COM_MSG_LEN, MPI_CHAR, MPI_ANY_SOURCE,
////	MPI_ANY_TAG, MPI_COMM_WORLD, &status);
////	puts(message_buff);
//	//TODO 初始化数据服务器的状态
//	msg_queue = alloc_msg_queue();
//	assert(msg_queue != NULL);
//
//	pthread_mutex_init(&mutex_message_buff, NULL);
//	pthread_mutex_init(&mutex_namespace, NULL);
//
//	pthread_create(&thread_master_server, NULL, master_server, NULL);
//	pthread_create(&thread_request_handler, NULL, request_handler, NULL);
//
//	pthread_join(thread_master_server, NULL);
//	pthread_join(thread_request_handler, NULL);
//
//	pthread_cond_init(&cond_request_queue, NULL);
//	//puts("=====master initialize end=====");
//}

void answer_client_create_file(common_msg_t *request){
	client_create_file *file_request = request->rest;

	basic_queue_t queue = master_data_server->opera->file_allocate_machine(master_data_server, file_request->file_size, BLOCK_SIZE);
	int malloc_result = queue == NULL ? 0 : 1;
	//TODO if communicate error
	MPI_Send(&malloc_result, 1, MPI_INT, request->source, CLIENT_INSTRUCTION_ANS_MESSAGE_TAG, MPI_COMM_WORLD);
	if(malloc_result == 0)
		return;

	ans_client_create_file *ans = (ans_client_create_file *)malloc(sizeof(ans_client_create_file));
	int ans_message_size = ceil(queue->current_size / LOCATION_MAX_BLOCK);
	int i;
	for(i = 1; i <= ans_message_size; i++){
		if(i != ans_message_size){
			ans->is_tail = 0;
			ans->block_num = LOCATION_MAX_BLOCK;
		}else{
			ans->is_tail = 1;
			ans->block_num = queue->current_size - (i - 1) * LOCATION_MAX_BLOCK;
		}
		//TODO need to generate a unique id
		//ans->generated_id
		ans->operation_code = CREATE_FILE_ANS_CODE;
		memcpy(ans->block_global_num, queue->elements + (i - 1) * LOCATION_MAX_BLOCK * queue->element_size, ans->block_num * queue->element_size);
		MPI_Send(ans, sizeof(ans_client_create_file), MPI_CHAR, request->source, CLIENT_INSTRUCTION_ANS_MESSAGE_TAG, MPI_COMM_WORLD);
	}
	free(ans);
}
