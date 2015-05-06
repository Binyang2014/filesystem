/*
 * master.h
 *
 *  Created on: 2015年1月4日
 *      Author: ron
 */

#ifndef SRC_MAIN_MASTER_H_
#define SRC_MAIN_MASTER_H_
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "namespace.h"
#include "conf.h"
#include "../tool/message.h"
#include "../global.h"
#include "../tool/error_log.h"

#define LOAD_FACTOR 0.5

/**
 * 数据服务器描述
 */
typedef struct data_server_des
{
	int id;								//机器id
	long last_time;						//上次交互时间
	uint32_t s_blocks_count;
	uint32_t s_free_blocks_count;
	uint_least8_t status;
	block *block_mem_map;
}data_server_des;

/**
 * 数据服务器
 */
typedef struct data_servers
{
	unsigned int server_count;
	data_server_des *data_server_list;
}data_servers;

/**
 * 请求队列
 * node of request queue, including request information from client and data server
 */
typedef struct request_node{
	unsigned short request_type;
	MPI_Status status;
	void *message;
	struct request_node *next;
}request_node;

/**
 * request queue
 */
typedef struct master_request_queue{
	int request_num;
	request_node *head;
	request_node *tail;
}master_request_queue;

/**
 * initialize request queue
 */
void init_queue();

void in_queue(request_node *request_param);

void init_master_server_info(int);

void init_data_server_node();

request_node* de_queue();

int request_is_empty();

request_node* malloc_request(char *buf, int size, MPI_Status *status);

file_location_des *maclloc_data_block(unsigned long file_size);

/**
 * use memcpy to implement copy of MPI_Status
 */
void mpi_status_assignment(MPI_Status *status, MPI_Status *s);

/* master initialize
 */
master_init();

/**
 * there are p_threads in the master machine
 */

/**
 * receive information from client and data server
 * information can classify as two kinds
 * 1. client operation code
 * 2. data server heart blood information
 */
static void* master_server(void *arg);

static void* request_handler(void *arg);

static void log_backup();

static void heart_blood();

static void namespace_control();

static int create_file();

pthread_t thread_master_server, thread_request_handler;
pthread_mutex_t mutex_message_buff, mutex_namespace, mutex_request_queue;
pthread_mutex_t mutex_send_message_buff;

pthread_cond_t cond_request_queue;

/**
 * receive message buff
 */
char message_buff[MAX_CMD_MSG_LEN];

/**
 * send message buff
 */
char send_message_buff[MAX_CMD_MSG_LEN];

static master_request_queue request_queue_list;

/*
 * data server
 */

static void data_server_init();

static data_servers *master_data_servers;

#endif /* SRC_MAIN_MASTER_H_ */
