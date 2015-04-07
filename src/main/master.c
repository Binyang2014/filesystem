/*
 * master.c
 *
 *  Created on: 2015年1月4日
 *      Author: ron
 */
#include "master.h"

void init_queue(){
	request_queue_list.head = request_queue_list.tail = NULL;
	request_queue_list.request_num = 0;
}

int request_is_empty(){
	return !request_queue_list.request_num;
}

void in_queue(request *request){
	request_queue_list.tail.next = request;
	request_queue_list.tail = request;
	request_queue_list.request_num++;
}

request* de_queue(){
	if(request_is_empty())
		return NULL;
	request *tmp_request = request_queue_list.head;
	request_queue_list.head = tmp_request.next;
	request_queue_list.request_num--;
	return tmp_request;
}

void mpi_status_assignment(MPI_Status *status, MPI_Status *s){
	memcpy(status, s, sizeof(MPI_Status));
}

request* malloc_request(char *buf, int size, MPI_Status *status){
	request *tmp_request = (request*)malloc(sizeof(request));
	tmp_request->message = (char *)malloc(size);
	strncpy(tmp_request->message, buf, size);
	mpi_status_assignment(tmp_request->status, status);
	return tmp_request;
}

void master_init()
{
	pthread_mutex_init(&mutex_message_buff, NULL);
	pthread_mutex_init(&mutex_namespace, NULL);
	pthread_create(&thread_master_namespace, NULL, namespace_control(), NULL);
	pthread_join(thread_master_namespace, NULL);
}

/**
 *	master的服务
 */
void master_server(){
	MPI_Status status;
	init_queue();
	while(1){
		pthread_mutex_lock(&mutex_message_buff);
		MPI_Recv(message_buff, MAX_CMD_MSG_LEN, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		if(status.MPI_ERROR){
			continue;
		}
		request *reqeust = malloc_request(message_buff, MAX_CMD_MSG_LEN);
		pthread_mutex_lock(&mutex_request_queue);
		in_queue(request);
		pthread_mutex_unlock(&mutex_request_queue);
		pthread_mutex_unlock(&mutex_message_buff);
	}
}

void request_handler(){
	request *request;
	while(1){
		pthread_mutex_lock(&mutex_request_queue);
		request = de_queue();
		pthread_mutex_unlock(&mutex_request_queue);
		if(request != NULL){
			unsigned short *operation_code = request->message;
			if(*operation_code == CREATE_FILE_CODE){
				create_file_structure *create = request->message;
				free(request->message);
			}
		}
	}
}

/**
 * master记录日志
 */
void log_backup(){

}

/*
 * master接受心跳信息
 */
void heart_blood(){

}

/**
 *	命名空间的操作
 */
void namespace_control(){
	while(1){

	}
}


