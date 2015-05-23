/*
 * master.c
 *
 *  Created on: 2015年1月4日
 *      Author: ron
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <mpi.h>
#include <assert.h>
#include "master.h"
#include "master_data_servers.h"
#include "namespace.h"
#include "../structure/basic_queue.h"
#include "../structure/namespace_code.h"
#include "../tool/message.h"
#include "../tool/syn_tool.h"
#include "../tool/errinfo.h"
#include "../global.h"

/*====================Private Prototypes====================*/
static namespace *master_namespace;
static basic_queue_t *message_queue;
static data_servers *master_data_servers;

static pthread_t *pthread_request_listener;
static pthread_t *pthread_request_handler;
//static pthread_t *pthread_damon_handler;

static char *receive_buf;
static char *send_buf;
static common_msg_t *msg_buff;
static common_msg_t *msg_pop_buff;
static common_msg_t *request_handle_buff;
static queue_syn_t *syn_message_queue;

static void set_common_msg(common_msg_t *msg, int source, char *message);
static void *master_server();

//static int master_create_file(char *file_name, unsigned long file_size);

/*====================private functions====================*/
static void set_common_msg(common_msg_t *msg, int source, char *message){
	unsigned short *operation_code = (unsigned short*)message;
	msg->operation_code = *operation_code;
	msg->source = source;
	memcpy(msg->rest, message, MAX_CMD_MSG_LEN);
}

static int answer_client_create_file(common_msg_t *request){
	client_create_file *file_request = (client_create_file *)(request->rest);
	//TODO the first is not going to provide any fault tolerance, but the name space modify should be temporary whenever is not confirmed
	int status = namespace_create_file(master_namespace , file_request->file_name);
	int malloc_result = 0;
	if(status != OPERATE_SECCESS){
		MPI_Send(&malloc_result, 1, MPI_INT, request->source, CLIENT_INSTRUCTION_ANS_MESSAGE_TAG, MPI_COMM_WORLD);
		err_ret("answer_client_create_file: name space create file failed, status = %d", status);
		return status;
	}
	basic_queue_t *queue = master_data_servers->opera->file_allocate_machine(master_data_servers, file_request->file_size, BLOCK_SIZE);
	malloc_result = (queue == NULL ? 0 : 1);
//	//TODO if communicate error
	MPI_Send(&malloc_result, 1, MPI_INT, request->source, CLIENT_INSTRUCTION_ANS_MESSAGE_TAG, MPI_COMM_WORLD);
	if(malloc_result == 0){
		//TODO cancel the modify to name space
		err_ret("master.c answer_client_create_file: allocate space fail for new file");
		return NO_ENOUGH_SPACE;
	}

	ans_client_create_file *ans = (ans_client_create_file *)malloc(sizeof(ans_client_create_file));
	if(ans == NULL){
		err_ret("master.c answer_client_create_file: allocate space fail for answer client create file buff");
		return NO_ENOUGH_SPACE;
	}
	int ans_message_size = ceil((double)queue->current_size / LOCATION_MAX_BLOCK);
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
	return 0;
}

/**
 *	master server
 *	receive message and put message into the message queue
 */
static void* master_server(void *arg) {
	MPI_Status status;
	while (1) {
		err_ret("master.c: master_server listening request");
		MPI_Recv(receive_buf, MAX_CMD_MSG_LEN, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		set_common_msg(msg_buff, status.MPI_SOURCE, receive_buf);
		syn_queue_push(message_queue, syn_message_queue, msg_buff);
		err_ret("master.c: master_server put message current_size = %d", message_queue->current_size);
	}
	return 0;
}

static void* request_handler(void *arg) {
	//int status;
	while (1) {
		syn_queue_pop(message_queue, syn_message_queue, msg_pop_buff);

		if (msg_pop_buff != NULL)
		{
			unsigned short operation_code = msg_pop_buff->operation_code;
			if (operation_code == CREATE_FILE_CODE) {
				//status =
				answer_client_create_file(msg_pop_buff);
			}
		}
	}
	return 0;
}

/*====================API Implementation====================*/
//TODO when shutdown the application, it must release all the memory resource
int master_init(){
	/*allocate necessary memory*/
	master_namespace = create_namespace(1024, 32);
	message_queue = alloc_basic_queue(sizeof(common_msg_t), -1);
	master_data_servers = data_servers_create(1024, 0.75, 10);
	queue_set_dup(message_queue, common_msg_dup);

	syn_message_queue = alloc_queue_syn();

	pthread_request_listener = (pthread_t *)malloc(sizeof(pthread_t));
	pthread_request_handler = (pthread_t *)malloc(sizeof(pthread_t));

	receive_buf = (char *)malloc(MAX_CMD_MSG_LEN);
	send_buf = (char *)malloc(MAX_CMD_MSG_LEN);
	msg_buff = (common_msg_t *)malloc(sizeof(common_msg_t));
	msg_pop_buff = (common_msg_t *)malloc(sizeof(common_msg_t));
	request_handle_buff = (common_msg_t *)malloc(sizeof(common_msg_t));
	//TODO check this
	if(master_namespace == NULL || message_queue == NULL || pthread_request_listener == NULL
			|| pthread_request_handler == NULL){
		master_destroy();
		return -1;
	}

	/*initialize server thread and critical resource lock*/
	pthread_create(pthread_request_listener, NULL, master_server, NULL);
	pthread_create(pthread_request_handler, NULL, request_handler, NULL);
	pthread_join(*pthread_request_handler, NULL);
	pthread_join(*pthread_request_listener, NULL);
	return 0;
}



//TODO dangerous operation, when shutdown the application
//TODO self-defined structure may be released by it's own free function
int master_destroy()
{
	free(master_namespace);
	free(message_queue);
	free(pthread_request_listener);
	free(pthread_request_handler);
	data_servers_destroy();
	destroy_queue_syn(syn_message_queue);
	return 0;
}
