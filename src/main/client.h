/*
 * clinet.h
 *
 *  Created on: 2015年1月4日
 *      Author: ron
 */
#include "mpi.h"
#include "opera_code_sturcture.h"
#include "conf.h"

#ifndef SRC_MAIN_CLIENT_H_
#define SRC_MAIN_CLIENT_H_


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

struct request_queue{
	enum client_request request_type;
	void *message_type;
	struct request_queue *next_request;
};
/**
 * 请求事件队列节点
 */

void client_server();

extern struct mpi_machine *master;

int queue_size;

struct request_queue *queue_head, *queue_tail;

/*****************instruction interface***********************/

static int create_file(char *file_name);

static int delete_file(char *dir_name);

static int create_dir(char *dir_name);



#endif /* SRC_MAIN_CLIENT_H_ */


#ifndef _CLIENT_MPI_COM
#define _CLIENT_MPI_CPM

#endif
