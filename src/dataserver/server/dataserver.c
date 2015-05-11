/*
 * Created on 2015.1.23
 * Author:binyang
 *
 * This file will finish all data server's work
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "dataserver.h"
#include "../structure/vfs_structure.h"
#include "../../tool/errinfo.h"
#include "../../structure/buffer.h"
#include "../../tool/threadpool.h"

//this is a demo, there are many things to add

//many kinds of locks

static data_server_t* data_server;

int get_current_imformation(data_server_t * server_imf)
{
	FILE* fp;
	char temp[100];
	char *parameter;

	fp = fopen("/proc/meminfo", "r");
	if(fp == NULL)
	{
		perror("can not get memory information");
		return 1;
	}

	fgets(temp, 100, fp);
	strtok(temp, " ");//分割字符串
	parameter = strtok(NULL, " ");
	server_imf->d_memory_used = strtoul(parameter, NULL, 10);

	fgets(temp, 100, fp);
	strtok(temp, " ");
	parameter = strtok(NULL, " ");
	server_imf->d_memory_free = strtoul(parameter, NULL, 10);
	server_imf->d_memory_used -= server_imf->d_memory_free;

	fclose(fp);
	return 0;
}

data_server_t* alloc_dataserver(total_size_t t_size, int dev_num)
{
	data_server = (data_server_t* )malloc(sizeof(data_server_t));
	if(data_server == NULL)
		err_sys("error while allocate data server");

	if( (data_server->d_super_block = vfs_init(t_size, dev_num)) == NULL )
		err_quit("error in init_dataserver function");

	//get_current_imformation(data_server)
	//It's just a joke, do not be serious
	data_server->files_buffer = (dataserver_file_t* )malloc(sizeof(dataserver_file_t)
			* D_FILE_BSIZE);

	//init f_arr_buff
	data_server->f_arr_buff = (vfs_hashtable_t* )malloc(sizeof(vfs_hashtable_t));
	data_server->f_arr_buff->hash_table_size = D_PAIR_BSIZE;
	data_server->f_arr_buff->blocks_arr = (unsigned int* )malloc(sizeof(unsigned int)
			* D_PAIR_BSIZE);
	data_server->f_arr_buff->chunks_arr = (unsigned long long*)malloc(sizeof(unsigned long long)
			* D_PAIR_BSIZE);

	data_server->m_data_buffer = (char* )malloc(sizeof(char) * D_DATA_BSIZE
			* MAX_DATA_MSG_LEN);
	//init threads pool
	//data_server->t_buffer = (pthread_t* )malloc(sizeof(pthread_t) * D_THREAD_SIZE);

	//init msg_cmd_buffer
	data_server->m_cmd_queue = alloc_msg_queue();
	if(data_server->m_cmd_queue == NULL)
	{
		return NULL;
	}

	//init many kinds of locks

	return data_server;
	//end of init
}

//receive cmd message from client or master
void* m_cmd_receive(void* msg_queue_arg)
{
	void* start_pos;
	common_msg_t* t_common_msg;
	msg_queue_t * msg_queue;
	mpi_status_t status;

	msg_queue = (msg_queue_t* )msg_queue_arg;
	t_common_msg = (common_msg_t* )malloc(sizeof(common_msg_t));

	while (1)
	{

#ifdef DATASERVER_COMM_DEBUG
		printf("The tid of this thread is %lu\n", (unsigned long)pthread_self());
		printf("I'm waiting for a message\n");
#endif
		//receive message from client
		start_pos = (char*) t_common_msg + COMMON_MSG_HEAD;
		d_mpi_cmd_recv(start_pos, &status);
		t_common_msg->source = status.source;
		//here should be changed, and new version of message queue isn't supports locks
		msg_queue->msg_op->push(msg_queue, t_common_msg);
	}

	free(t_common_msg);
	return NULL;
}

/*=================================resolve message=========================================*/

static int init_rw_event_handler(event_handler_t* event_handler,
		common_msg_t* common_msg, int flag)
{
	int i;
	unsigned int chunks_count;
	buffer_t* t_buff;
	msg_r_ctod_t* read_msg = NULL;
	msg_w_ctod_t* write_msg = NULL;
	event_handler->spcical_struct = data_server;

	//get buffer structure for event handler
	 if( (event_handler->event_buffer = get_series_buffer(data_server, 5)) == -1 )
		 return -1;
	t_buff = event_handler->event_buffer;

	//decide use which structure
	if(flag == MSG_READ)
	{
		read_msg = (msg_r_ctod_t* )MSG_COMM_TO_CMD(common_msg);
		chunks_count = read_msg->chunks_count;
	}
	else
	{
		write_msg = (msg_w_ctod_t* )MSG_COMM_TO_CMD(common_msg);
		chunks_count = write_msg->chunks_count;
	}

	//get specific buffer for event
	if( (t_buff->buffer = get_common_msg_buff(data_server, common_msg)) == NULL )
		return -1;
	t_buff = t_buff->next;
	if( (t_buff->buffer = get_msg_buffer(data_server)) == NULL )
		return -1;
	t_buff = t_buff->next;
	if( (t_buff->buffer = get_data_buffer(data_server)) == NULL )
		return -1;
	t_buff = t_buff->next;
	if( (t_buff->buffer = get_file_info(data_server)) == NULL )
		return -1;
	t_buff = t_buff->next;
	if( (t_buff->buffer = get_f_arr_buff(data_server)) == NULL )
		return -1;

	return 0;
}

void* m_resolve(event_handler_t* event_handler, void* msg_queue)
{
	//this variable allocate in stack. may be each thread need one common message
	static common_msg_t t_common_msg;
	msg_queue_t* t_msg_queue = msg_queue;
	unsigned short operation_code;

#ifdef DATASERVER_COMM_DEBUG
	printf("In m_resolve function\n");
#endif

	//here should be changed, and new version of message queue isn't supports locks
	t_msg_queue->msg_op->pop(msg_queue, &t_common_msg);
	operation_code = t_common_msg->operation_code;

	//this function should solve how to initiate event handler, each thread has its' own
	//event handler and we do not need to care about it
	switch(operation_code)
	{
	case MSG_READ:
		init_rw_event_handler(event_handler, &t_common_msg, MSG_READ);
		//invoke a thread to excuse
		return d_read_handler;
	case MSG_WRITE:
		init_rw_event_handler(event_handler, &t_common_msg, MSG_WRITE);
		//invoke a thread to excuse
		return d_write_handler;
	default:
		return NULL;
	}
	return NULL;
}
