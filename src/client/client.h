/*
 * clinet.h
 *
 *  Created on: 2015年1月4日
 *      Author: ron
 */


#ifndef SRC_MAIN_CLIENT_H_
#define SRC_MAIN_CLIENT_H_

#define FILE_BLOCK_SIZE 4096

typedef struct request_queue{
	//enum client_request request_type;
	void *message_type;
	struct request_queue *next_request;
}request_queue;

/**
 * initialize the client
 */
void client_init();

void client_server();

int queue_size;

struct request_queue *queue_head, *queue_tail;

/*****************instruction interface***********************/

//与master通信
/********communicate with master************/
static int clent_create_file(char *file_path, char *file_name);

static void handle_create_file_ans();

void send_data(char *file_name);

char *rec_buf, *send_buf;

static mas_ans_cli_crea_file* create_file_ans;

//与数据服务器通信
/***********communicate with data server********/
int client_data_server_create_file(unsigned long file_size);


//提供给本地程序的服务接口
/***********client service****************************/


#endif /* SRC_MAIN_CLIENT_H_ */
