/*
 * Created on 2015.1.23
 * Author:binyang
 *
 * This file will finish all data server's work
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>
#include <pthread.h>
#include <unistd.h>
#include "dataserver.h"
#include "dataserver_buff.h"
#include "dataserver_handler.h"
#include "../structure/vfs_structure.h"
#include "../../structure/basic_list.h"
#include "../../structure/basic_queue.h"
#include "../../tool/errinfo.h"
#include "../../tool/syn_tool.h"
#include "../../tool/threadpool.h"

//this is a demo, there are many things to add

//many kinds of locks

static data_server_t* data_server;
static void* heart_beat(void* args);

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

	//get_current_imformation(data_server);
	data_server->machine_id = dev_num;
	//init buffer
	set_data_server_buff(data_server, THREAD_POOL_SIZE);
	//init msg_cmd_buffer
	data_server->m_cmd_queue = alloc_basic_queue(COMMON_MSG_LEN, 0);
	//init threads pool
	data_server->thread_pool = alloc_thread_pool(THREAD_POOL_SIZE, data_server->m_cmd_queue);
	//init queue_syn_tool
	data_server->queue_syn = alloc_queue_syn();
	//init event_handler
	data_server->event_handler = alloc_event_handler_set(data_server->thread_pool,
			m_resolve);
	//init many kinds of locks
	return data_server;
	//end of init
}

//receive cmd message from client or master
void m_cmd_receive()
{
	void* start_pos;
	basic_queue_t * msg_queue;
	static common_msg_t t_common_msg;
	static mpi_status_t status;

	msg_queue = data_server->m_cmd_queue;

#ifdef DATASERVER_COMM_DEBUG
	printf("The tid of this thread is %lu\n", (unsigned long)pthread_self());
	printf("I'm waiting for a message\n");
#endif
	//receive message from client
	start_pos = (char*) (&t_common_msg) + COMMON_MSG_HEAD;
	d_mpi_cmd_recv(start_pos, &status);
	t_common_msg.source = status.source;
	//use syn_queue_push here
	syn_queue_push(msg_queue, data_server->queue_syn, &t_common_msg);
}

/*=================================resolve message=========================================*/

static int init_rw_event_handler(event_handler_t* event_handler,
		common_msg_t* common_msg, int flag)
{
	unsigned int chunks_count;
	list_t* t_buff_list;
	list_node_t* t_buff;
	list_iter_t* iter;
	msg_r_ctod_t* read_msg = NULL;
	msg_w_ctod_t* write_msg = NULL;
	event_handler->spcical_struct = data_server;

	//get buffer structure for event handler
	 if( (event_handler->event_buffer_list = get_buffer_list(data_server, 5)) == NULL )
		 return -1;
	t_buff_list = event_handler->event_buffer_list;

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

	//get iterator for the list
	iter = t_buff_list->list_ops->list_get_iterator(t_buff_list, 0);

	t_buff = t_buff_list->list_ops->list_next(iter);
	//get specific buffer for event
	if( (t_buff->value = get_common_msg_buff(data_server)) == NULL )
	{
		t_buff_list->list_ops->list_release_iterator(iter);
		return -1;
	}
	memcpy(t_buff->value, common_msg, sizeof(common_msg_t));

#ifdef DATASERVER_COMM_DEBUG
	printf("the cmmon_msg is %p\n", t_buff->value);
#endif

	t_buff = t_buff_list->list_ops->list_next(iter);
	if( (t_buff->value = get_reply_msg_buff(data_server)) == NULL )
	{
		t_buff_list->list_ops->list_release_iterator(iter);
		return -1;
	}

#ifdef DATASERVER_COMM_DEBUG
	printf("the reply_msg_buff is %p\n", t_buff->value);
#endif

	t_buff = t_buff_list->list_ops->list_next(iter);
	if( (t_buff->value = get_data_buff(data_server)) == NULL )
	{
		t_buff_list->list_ops->list_release_iterator(iter);
		return -1;
	}

#ifdef DATASERVER_COMM_DEBUG
	printf("the data_buff is %p\n", t_buff->value);
#endif

	t_buff = t_buff_list->list_ops->list_next(iter);
	if( (t_buff->value = get_file_info_buff(data_server)) == NULL )
	{
		t_buff_list->list_ops->list_release_iterator(iter);
		return -1;
	}

#ifdef DATASERVER_COMM_DEBUG
	printf("the file_info_buff is %p\n", t_buff->value);
#endif

	t_buff = t_buff_list->list_ops->list_next(iter);
	if( (t_buff->value = get_f_arr_buff(data_server, chunks_count)) == NULL )
	{
		t_buff_list->list_ops->list_release_iterator(iter);
		return -1;
	}

#ifdef DATASERVER_COMM_DEBUG
	printf("the f_arr_buff is %p\n", t_buff->value);
	printf("the buffer size is %d\n", ((vfs_hashtable_t*)t_buff->value)->hash_table_size);
	printf("the array buffer is %p\n", ((vfs_hashtable_t*)t_buff->value)->blocks_arr);
	printf("the chunks buffer is %p\n", ((vfs_hashtable_t*)t_buff->value)->chunks_arr);
#endif

	t_buff_list->list_ops->list_release_iterator(iter);
	return 0;
}

void* m_resolve(event_handler_t* event_handler, void* msg_queue)
{
	//this variable allocate in stack. may be each thread need one common message
	static common_msg_t t_common_msg;
	basic_queue_t* t_msg_queue = msg_queue;
	unsigned short operation_code;
	int error;

#ifdef DATASERVER_COMM_DEBUG
	printf("In m_resolve function\n");
#endif

	//access queue use syn_queue_push
	syn_queue_pop(t_msg_queue, data_server->queue_syn, &t_common_msg);
	operation_code = t_common_msg.operation_code;

	//this function should solve how to initiate event handler, each thread has its' own
	//event handler and we do not need to care about it
	switch(operation_code)
	{
	case MSG_READ:
		error = init_rw_event_handler(event_handler, &t_common_msg, MSG_READ);
		if(error == -1)
			err_quit("error when allocate buffer");
		//invoke a thread to excuse
		return d_read_handler;
	case MSG_WRITE:
		error = init_rw_event_handler(event_handler, &t_common_msg, MSG_WRITE);
		if(error == -1)
			err_quit("error when allocate buffer");
		//invoke a thread to excuse
		return d_write_handler;
	default:
		return NULL;
	}
	return NULL;
}

void *dataserver_run(void *arg)
{
	data_server_t* dateserver = (data_server_t*)arg;
	char* msg;
	pthread_t tid;

	msg = (char*)malloc(sizeof(char) * MAX_CMD_MSG_LEN);
	dateserver->thread_pool->tp_ops->start(dateserver->thread_pool, dateserver->event_handler);
	pthread_create(&tid, NULL, heart_beat, msg);
	for(;;)
	{
		m_cmd_receive();
	}
	return 0;
}

void* heart_beat(void* msg)
{
	for(;;)
	{
		d_server_heart_blood_t* heart_beat_msg;
		heart_beat_msg = (d_server_heart_blood_t*)msg;
		heart_beat_msg->operation_code = D_M_HEART_BLOOD_CODE;
		heart_beat_msg->id = data_server->machine_id;
		//send heart beat to master
		d_mpi_cmd_send(heart_beat_msg, 0, 0);
		//printf("heart beat coming\n");
		sleep(1);
	}
	return NULL;
}
