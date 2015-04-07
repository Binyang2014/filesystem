/*
 * master.h
 *
 *  Created on: 2015年1月4日
 *      Author: ron
 */

#ifndef SRC_MAIN_MASTER_H_
#define SRC_MAIN_MASTER_H_
#include <pthread.h>
#include <mpi.h>
#include "opera_code_sturcture.h"
#include "namespace.h"
#include "conf.h"
#include "../tool/message.h"
#include "../global.h"


typedef struct{

}data_server_des;

/**
 * status of all data servers
 */
data_server_des *data_servers;

/**
 * node of request queue, including request information from client and data server
 */
typedef struct{
	unsigned short request_type;
	MPI_Status status;
	void *message;
	request *next;
}request;

/**
 * request queue
 */
typedef struct{
	int request_num;
	request *head;
	request *tail;
}master_request_queue;

static void init_queue();

static void in_queue();

static request* de_queue();

static int request_is_empty();

static request* malloc_request(char *buf, int size);

/**
 * use memcpy to implement copy of MPI_Status
 */
static void mpi_status_assignment(MPI_Status *status, MPI_Status *s);

/* master initialize
 */
static void master_init();

static void master_server();

static void log_backup();

static void heart_blood();

static void namespace_control();

static int create_file();

pthread_t thread_master_log_backup, thread_master_namespace, thread_master_heart;
pthread_mutex_t mutex_message_buff, mutex_namespace, mutex_request_queue;

char message_buff[MAX_CMD_MSG_LEN];

static master_request_queue request_queue_list;

#endif /* SRC_MAIN_MASTER_H_ */
