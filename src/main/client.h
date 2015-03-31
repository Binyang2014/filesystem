/*
 * clinet.h
 *
 *  Created on: 2015年1月4日
 *      Author: ron
 */
#include "mpi.h"
#include "opera_code_sturcture.h"
#include "conf.h"
#include "../tool/message.h"
#include "../tool/file_tool.h"

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

extern struct mpi_machine *master;

int queue_size;

struct request_queue *queue_head, *queue_tail;

/*****************instruction interface***********************/

static int create_file(char *file_path, char *file_name);

static int create_new_file(char *file_name);

static int delete_file(char *dir_name);

static int create_dir(char *dir_name);

static void send_data(char *file_name);

#endif /* SRC_MAIN_CLIENT_H_ */
