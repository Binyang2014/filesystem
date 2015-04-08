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

char file_buf[FILE_BLOCK_SIZE];
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

void client_init(){
//	char b[MAX_COM_MSG_LEN] = "hello world";
//	MPI_Send(b, MAX_COM_MSG_LEN, MPI_CHAR, 0, 1, MPI_COMM_WORLD);
	send_buf = (char*)malloc(MAX_CMD_MSG_LEN);
	rec_buf = (char*)malloc(MAX_CMD_MSG_LEN);
	if(send_buf == NULL || rec_buf == NULL){
		log_error("there is not enough stack space");
		return;
	}
	clent_create_file("/home/ron/test/test_struct.c", "/hello");
}

void send_data(char *file_name) {
	FILE *fp = fopen(file_name, "r");
	long length = file_size(file_name);
	int block_num = ceil((double)length / FILE_BLOCK_SIZE);
	int i = 0, j = 0, read_size;
	for (; i <= block_num - 1; i++) {
		if(i != block_num - 1){
			read_size = fread(file_buf, sizeof(char), FILE_BLOCK_SIZE, fp);
			fseek(fp, FILE_BLOCK_SIZE, FILE_BLOCK_SIZE * i);
		}else{
			read_size = fread(file_buf, sizeof(char), length - (block_num - 1) * FILE_BLOCK_SIZE, fp);
//			for(j = 0; j != length - (block_num - 1) * FILE_BLOCK_SIZE; j++)
//				putchar(file_buf[j]);
		}
	}
	fclose(fp);
}

int clent_create_file(char *file_path, char *file_name){
	log_info("client_create_file start");
	long file_length = file_size(file_path);
	int result;
	char tmp[5096];
	MPI_Status status;
	if(file_length == -1)
		return -1;
	create_file_structure message;
	message.instruction_code = CREATE_FILE_CODE;
	message.file_size = file_length;
	strcpy(message.file_name, file_name);
	log_info("send message start");

	MPI_Send((void*)&message, sizeof(message), MPI_CHAR, master.rank, CLIENT_INSTRCTION_MESSAGE, MPI_COMM_WORLD);
	MPI_Recv(tmp, 4096, MPI_CHAR, master.rank, CLIENT_INSTRUCTION_ANS_MESSAGE, MPI_COMM_WORLD, &status);
	log_info("client_create_file end");
	return 0;
}

int create_new_file(char *file_name){

}




