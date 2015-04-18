/*
 * clinet.h
 *
 *  Created on: 2015年1月4日
 *      Author: ron
 */
#include <math.h>
#include <string.h>
#include "mpi.h"
#include "opera_code_sturcture.h"
#include "conf.h"
#include "../tool/message.h"
#include "../tool/file_tool.h"
#include "../tool/error_log.h"

#ifndef SRC_MAIN_CLIENT_H_
#define SRC_MAIN_CLIENT_H_

#define FILE_BLOCK_SIZE 4096

enum client_request{
	CREATE_FILE,
	CREATE_DIR,
	DEL_FILE,
	DEL_DIR,
	RENAME_FILE,
	RENAME_DIR,
	MV_FILE,
	MV_DIR
};

typedef struct request_queue{
	enum client_request request_type;
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

/**
 * create file
 */
static int clent_create_file(char *file_path, char *file_name);

static void handle_create_file_ans();

void send_data(char *file_name);

char *rec_buf, *send_buf;

static mas_ans_cli_crea_file* create_file_ans;

#endif /* SRC_MAIN_CLIENT_H_ */
