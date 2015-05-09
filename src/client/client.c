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
#include <math.h>
#include <string.h>
#include "mpi.h"
#include "conf.h"
#include "../tool/message.h"
#include "../tool/file_tool.h"

char file_buf[FILE_BLOCK_SIZE];
char message_content[CLIENT_MASTER_MESSAGE_SIZE];

void client_init() {
	send_buf = (char*) malloc(MAX_CMD_MSG_LEN);
	if (send_buf == NULL) {
		log_error("there is not enough stack space");
		return;
	}

	rec_buf = (char*) malloc(MAX_CMD_MSG_LEN);
	if(rec_buf == NULL){
		free(send_buf);
		log_error("there is not enough stack space");
		return;
	}

	clent_create_file("/home/ron/test/test_struct.c", "/hello");
}

void send_data(char *file_name) {
	FILE *fp = fopen(file_name, "r");
	long length = file_size(file_name);
	int block_num = ceil((double) length / FILE_BLOCK_SIZE);
	int i = 0, j = 0, read_size;
	for (; i <= block_num - 1; i++) {
		if (i != block_num - 1) {
			read_size = fread(file_buf, sizeof(char), FILE_BLOCK_SIZE, fp);
			fseek(fp, FILE_BLOCK_SIZE, FILE_BLOCK_SIZE * i);
		} else {
			read_size = fread(file_buf, sizeof(char),
					length - (block_num - 1) * FILE_BLOCK_SIZE, fp);
//			for(j = 0; j != length - (block_num - 1) * FILE_BLOCK_SIZE; j++)
//				putchar(file_buf[j]);
		}
	}
	fclose(fp);
}

int clent_create_file(char *file_path, char *file_name) {
	//log_info("client_create_file start");
	long file_length = file_size(file_path);
	if (file_length == -1)
		return -1;

	int result;

	char tmp_buf[CLIENT_MASTER_MESSAGE_SIZE];
	int master_malloc_result;

	MPI_Status status;

	create_file_structure message;
	message.instruction_code = CREATE_FILE_CODE;
	message.file_size = file_length;
	strcpy(message.file_name, file_name);
	//log_info("send message start");

	MPI_Send((void*) &message, sizeof(message), MPI_CHAR, master.rank, CLIENT_INSTRCTION_MESSAGE_TAG, MPI_COMM_WORLD);
	MPI_Recv(&master_malloc_result, 1, MPI_INT, master.rank, CLIENT_INSTRUCTION_ANS_MESSAGE_TAG, MPI_COMM_WORLD, &status);
	if(master_malloc_result == 1){
		while(1){
			MPI_Recv(tmp_buf, CLIENT_MASTER_MESSAGE_SIZE, MPI_CHAR, master.rank, CLIENT_INSTRUCTION_ANS_MESSAGE_TAG, MPI_COMM_WORLD, &status);
		}
	}
	//TODO 如果master成功分配内存，那么此时应该等待接收data server机器信息
	//log_info("client_create_file end");
	return 0;
}




