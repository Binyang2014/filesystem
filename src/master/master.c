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
static queue_syn_t *syn_message_queue;
static char *master_cmd_buff;
static common_msg_t *master_msg_buff;
static pthread_t *master_threads;
static data_servers *master_data_servers;

//static pthread_t *pthread_damon_handler;

typedef enum thread_buff{
	MASTER_REQUEST_LISTENER,
	MASTER_REQUEST_HANDLER,
	MASTER_NAMESPACE_HANDLER
}thread_buff;

typedef enum cmd_buff{
	MASTER_CMD_SEND_BUFF = 0,
	MASTER_CMD_RECV_BUFF = 1 * MAX_CMD_MSG_LEN
}cmd_buff;

typedef enum msg_buff{
	MASTER_MSG_SEND_BUFF,
	MASTER_MSG_RECV_BUFF,
	MASTER_MSG_TO_RUN,
	MASTER_REQUEST_MSG_BUFF
}msg_buff;

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
	basic_queue_t *queue = master_data_servers->opera->file_allocate_machine(master_data_servers, file_request->file_size, MAX_COUNT_DATA);
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
	int i = 1;
	for(; i <= ans_message_size; i++){
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
		//printf("-------ans_size=====%d\n", ans->block_num);
		MPI_Send(ans, sizeof(ans_client_create_file), MPI_CHAR, request->source, CLIENT_INSTRUCTION_ANS_MESSAGE_TAG, MPI_COMM_WORLD);
	}
	free(ans);
	return 0;
}

static int heart_blood(data_servers *servers, common_msg_t *msg, time_t time){
	d_server_heart_blood_t *cmd = (d_server_heart_blood_t *)msg->rest;
	//int server_id, data_server_status status, time_t time
	//err_ret("HEART_BLOOD.C: heart_blood listening request");
	return servers->opera->heart_blood(servers, cmd->id - 1,  SERVER_AVAILABLE, time);
}

/**
 *	master server
 *	receive message and put message into the message queue
 */
static void* master_server(void *arg) {
	mpi_status_t status;
	common_msg_t *m = master_msg_buff + MASTER_MSG_RECV_BUFF;
	char *c = master_cmd_buff + MASTER_CMD_RECV_BUFF;
	while (1) {
		//err_ret("master.c: master_server listening request");
		m_mpi_cmd_recv(c, &status);
		set_common_msg(m, status.source, c);
		syn_queue_push(message_queue, syn_message_queue, m);
		//err_ret("master.c: master_server put message current_size = %d", message_queue->current_size);
	}
	return 0;
}

static void* request_handler(void *arg) {
	//int status;
	common_msg_t *cmd = master_msg_buff + MASTER_MSG_TO_RUN;
	while (1) {
		syn_queue_pop(message_queue, syn_message_queue, cmd);

		if (cmd != NULL)
		{
			unsigned short operation_code = cmd->operation_code;
			switch(operation_code)
			{
				case CREATE_FILE_CODE:
					answer_client_create_file(cmd);
					continue;
				case D_M_HEART_BLOOD_CODE:
					heart_blood(master_data_servers, cmd, time(NULL));
					continue;
				default:
					continue;
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
	master_cmd_buff = (char *)malloc(MAX_CMD_MSG_LEN * 4);
	master_msg_buff = (common_msg_t *)malloc(sizeof(common_msg_t) * 4);
	master_threads = (pthread_t *)malloc(sizeof(pthread_t) * 4);
	master_data_servers = data_servers_create(1024, 0.75, 1500);
	syn_message_queue = alloc_queue_syn();

	//TODO check this
	if(master_namespace == NULL || message_queue == NULL || master_cmd_buff == NULL
			|| master_msg_buff == NULL || master_threads == NULL || master_data_servers == NULL
			|| syn_message_queue == NULL){
		master_destroy();
		return -1;
	}

	queue_set_dup(message_queue, common_msg_dup);

	/*initialize server thread and critical resource lock*/
	pthread_create(master_threads + MASTER_REQUEST_LISTENER, NULL, master_server, NULL);
	pthread_create(master_threads + MASTER_REQUEST_HANDLER, NULL, request_handler, NULL);
	pthread_join(*(master_threads + MASTER_REQUEST_LISTENER), NULL);
	pthread_join(*(master_threads + MASTER_REQUEST_HANDLER), NULL);
	return 0;
}



//TODO dangerous operation, when shutdown the application
//TODO self-defined structure may be released by it's own free function
int master_destroy()
{
	free(master_namespace);
	destroy_basic_queue(message_queue);
	free(master_cmd_buff);
	free(master_msg_buff);
	free(master_threads);
	data_servers_destroy();
	destroy_queue_syn(syn_message_queue);
	return 0;
}
