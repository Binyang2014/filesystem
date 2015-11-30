/*
 * Created on 2015.1.23
 * Author:binyang
 * Modified on 2015.7.31
 * This file will finish all data server's work
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include "dataserver.h"
#include "dataserver_buff.h"
#include "dataserver_handler.h"
#include "basic_list.h"
#include "log.h"
#include "zmalloc.h"
#include "data_master_request.h"

//this is a demo, there are many things to add
//initial flag
#define MSG_READ 0000
#define MSG_WRITE 0001
//many kinds of locks

static data_server_t* data_server;
//static void* heart_beat(void* args);

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
	data_server = (data_server_t* )zmalloc(sizeof(data_server_t));
	if(data_server == NULL)
	{
		err_sys("error while allocate data server");
	}

	if( (data_server->d_super_block = vfs_init(t_size, dev_num)) == NULL )
	{
		err_quit("error in init_dataserver function");
	}

	//get_current_imformation(data_server);
	data_server->machine_id = dev_num;
	//init buffer
	set_data_server_buff(data_server, THREAD_POOL_SIZE);

	//init rpc server
	data_server->rpc_server = create_rpc_server(THREAD_POOL_SIZE, MSG_QUEUE_SIZE, dev_num, m_resolve);

	//init many kinds of locks
	return data_server;
	//end of init
}

void destroy_dataserver(data_server_t* data_server)
{
	destroy_rpc_server(data_server->rpc_server);
	free_data_server_buff(data_server);
	vfs_destroy(data_server->d_super_block);
	zfree(data_server);
}
/*=================================resolve message=========================================*/

static int init_rw_event_handler(event_handler_t* event_handler,
		common_msg_t* common_msg, int flag)
{
	unsigned int chunks_count;
	list_t* t_buff_list;
	list_node_t* t_buff;
	list_iter_t* iter;
	read_c_to_d_t* read_msg = NULL;
	write_c_to_d_t* write_msg = NULL;
	event_handler->special_struct = data_server;

	//get buffer structure for event handler
	 if( (event_handler->event_buffer_list = get_buffer_list(data_server, 4)) == NULL )
		 return -1;
	t_buff_list = event_handler->event_buffer_list;

	//decide use which structure
	if(flag == MSG_READ)
	{
		read_msg = (read_c_to_d_t* )MSG_COMM_TO_CMD(common_msg);
		chunks_count = read_msg->chunks_count;
	}
	else
	{
		write_msg = (write_c_to_d_t* )MSG_COMM_TO_CMD(common_msg);
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
	log_write(LOG_DEBUG, "the cmmon_msg is %p", t_buff->value);
#endif

	t_buff = t_buff_list->list_ops->list_next(iter);
	if( (t_buff->value = get_data_buff(data_server)) == NULL )
	{
		t_buff_list->list_ops->list_release_iterator(iter);
		return -1;
	}

#ifdef DATASERVER_COMM_DEBUG
	log_write(LOG_DEBUG, "the data_buff is %p", t_buff->value);
#endif

	t_buff = t_buff_list->list_ops->list_next(iter);
	if( (t_buff->value = get_file_info_buff(data_server)) == NULL )
	{
		t_buff_list->list_ops->list_release_iterator(iter);
		return -1;
	}

#ifdef DATASERVER_COMM_DEBUG
	log_write(LOG_DEBUG, "the file_info_buff is %p\n", t_buff->value);
#endif

	t_buff = t_buff_list->list_ops->list_next(iter);
	if( (t_buff->value = get_f_arr_buff(data_server, chunks_count)) == NULL )
	{
		t_buff_list->list_ops->list_release_iterator(iter);
		return -1;
	}

#ifdef DATASERVER_COMM_DEBUG
	log_write(LOG_DEBUG, "the f_arr_buff is %p", t_buff->value);
	log_write(LOG_DEBUG, "the buffer size is %d", ((vfs_hashtable_t*)t_buff->value)->hash_table_size);
	log_write(LOG_DEBUG, "the array buffer is %p", ((vfs_hashtable_t*)t_buff->value)->blocks_arr);
	log_write(LOG_DEBUG, "the chunks buffer is %p", ((vfs_hashtable_t*)t_buff->value)->chunks_arr);
#endif

	t_buff_list->list_ops->list_release_iterator(iter);
	return 0;
}

void* m_resolve(event_handler_t* event_handler, void* msg_queue)
{
	//this variable allocate in stack. may be each thread need one common message
	static common_msg_t t_common_msg;
	syn_queue_t* syn_queue = msg_queue;
	uint16_t operation_code;
	int error;

	//access queue use syn_queue_push
#if DATA_SERVER
	log_write(LOG_DEBUG, "data server m_resolve queue is %d and t_common_msg = %d", syn_queue, t_common_msg);
#endif
	syn_queue->op->syn_queue_pop(syn_queue, &t_common_msg);
	operation_code = t_common_msg.operation_code;

#ifdef DATASERVER_COMM_DEBUG
	log_write(LOG_DEBUG, "In m_resolve function and operatopn code is %d\n", operation_code);
#endif


	//this function should solve how to initiate event handler, each thread has its' own
	//event handler and we do not need to care about it
	switch(operation_code)
	{
	case C_D_READ_BLOCK_CODE:
		error = init_rw_event_handler(event_handler, &t_common_msg, MSG_READ);
		if(error == -1)
		{
			err_quit("error when allocate buffer");
		}
		//invoke a thread to excuse
		return d_read_handler;

	case C_D_WRITE_BLOCK_CODE:
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
	data_server_t* data_server = (data_server_t*)arg;
	//char* msg;

	log_write(LOG_INFO, "=====data server has started=====");
	//msg = (char*)malloc(sizeof(char) * MAX_CMD_MSG_LEN);
	data_server->rpc_server->op->server_start(data_server->rpc_server);
//	pthread_create(&tid, NULL, heart_beat, msg);
#if DATA_SERVER_DEABUG
	log_write(LOG_DEBUG, "server start success and server rank == %d\n", data_server->machine_id);
#endif
	return 0;
}

//void* heart_beat(void* msg)
//{
//	for(;;)
//	{
//		d_server_heart_blood_t* heart_beat_msg;
//		heart_beat_msg = (d_server_heart_blood_t*)msg;
//		heart_beat_msg->operation_code = D_M_HEART_BLOOD_CODE;
//		heart_beat_msg->id = data_server->machine_id;
//		//send heart beat to master
//		d_mpi_cmd_send(heart_beat_msg, 0, 0);
//		//printf("heart beat coming\n");
//		sleep(1);
//	}
//	return NULL;
//}
