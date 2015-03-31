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

char message_content[CLIENT_MASTER_MESSAGE_SIZE];

//int queue_empty(){
//	return queue_size == 0;
//}
//
//void in_queue(const struct request_queue *request){
//	queue_size++;
//	queue_tail->next_request = request;
//	queue_tail=request;
//}
//
//struct request_queue* de_queue(){
//	if(queue_empty())
//		return 0;
//	queue_size--;
//	struct request_queue *head = queue_head;
//	queue_head = queue_head->next_request;
//	return head;
//}
//
//void init(){
//	queue_size = 0;
//	queue_tail = queue_head = 0;
//}


//void client_server() {
//	init();
//	struct request_queue *request;
//	int send_status = MPI_Send((const void*) request,
//			sizeof(struct request_queue), MPI_BYTE, master->rank, 1,
//			master->comm);
//}

int create_file(char *file_path, char *file_name){
	long file_length = file_size(file_path);
	int result;
	if(file_length == -1)
		return -1;
	create_file_structure message;
	message.instruction_code = create_file_code;
	message.file_size = file_length;
	strcpy(message.file_name, file_name);
	MPI_Send((void*)message, sizeof(message), MPI_CHAR, master->rank, CLIENT_INSTRCTION_MESSAGE, master->comm);
	//MPI_Recv(&result, 1, MPI_INT, )
}

int create_new_file(char *file_name){

}




