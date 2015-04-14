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

//this is a demo, there are many things to add

static data_server_t* data_server;
//many kinds of locks
static pthread_rwlock_t msg_queue_rw_lock;

void* m_cmd_receive(void* msg_queue_arg)
{
	int offset;
	char* start_pos;
	common_msg_t* t_common_msg;
	msg_queue_t * msg_queue;
	MPI_Status status;

	msg_queue = (msg_queue_t* )msg_queue_arg;
	while (1)
	{
		//read lock, we also need signal here. I will add it later
		pthread_rwlock_rdlock(&msg_queue_rw_lock);
		if (!Q_FULL(msg_queue->head_pos, msg_queue->tail_pos))
		{
			offset = msg_queue->tail_pos;
			//read unlock
			pthread_rwlock_unlock(&msg_queue_rw_lock);

#ifdef DATASERVER_COMM_DEBUG
			printf("The tid of this thread is %lu\n", (unsigned long)pthread_self());
			printf("I'm waiting for a message\n");
#endif

			//receive message first
			t_common_msg = msg_queue->msg + offset;
			start_pos = (char*) t_common_msg + COMMON_MSG_HEAD;
			MPI_Recv(start_pos, MAX_CMD_MSG_LEN, MPI_CHAR, MPI_ANY_SOURCE,
					D_MSG_CMD_TAG,
					MPI_COMM_WORLD, &status);

#ifdef DATASERVER_COMM_DEBUG
			printf_msg_status(&status);
#endif

			t_common_msg->source = status.MPI_SOURCE;
			t_common_msg->unique_tag = status.MPI_TAG;
			//finish receive write lock
			pthread_rwlock_wrlock(&msg_queue_rw_lock);
			msg_queue->tail_pos = (msg_queue->tail_pos + 1) % D_MSG_BSIZE;
			msg_queue->current_size = msg_queue->current_size + 1;
			//write unlock
			pthread_rwlock_unlock(&msg_queue_rw_lock);
		}
		else
		{
			pthread_rwlock_unlock(&msg_queue_rw_lock);
			//Maybe if the space is no big enough, I should realloc if and make it a bigger heap

#ifdef DATASERVER_COMM_DEBUG
			err_msg("The message queue already full!!!");
			return NULL;
#endif
		}
	}
}

static void* d_read(data_server_t* this, common_msg_t* common_msg)
{
	int source, tag;
	char* buff;
	void* msg_buff;
	msg_r_ctod_t* read_msg;
	msg_for_rw_t* file_info;

	//init basic information, and it just for test now!!
	read_msg = (msg_r_ctod_t* )MSG_COMM_TO_CMD(common_msg);
	source = common_msg->source;
	tag = read_msg->unique_tag;
	msg_buff = (void* )malloc(MAX_CMD_MSG_LEN);//may be should use a message queue here
	buff = this->m_data_buffer;
	file_info = (msg_for_rw_t* )malloc(sizeof(msg_for_rw_t));
	file_info->offset = read_msg->offset;
	file_info->count = read_msg->read_len;
	file_info->file = init_vfs_file(this->d_super_block, this->files_buffer,
			this->f_arr_buff, VFS_READ);
	if( m_read_handler(source, tag, file_info, buff, msg_buff) == -1 )
	{
		free(msg_buff);
		free(file_info);
		return NULL;
	}
	printf("It's OK here\n");
	//free(msg_buff);
	//free(file_info);
	return NULL;
}

static void* d_write(data_server_t* this, common_msg_t* common_msg)
{
	int source, tag;
	char* buff;
	void* msg_buff;
	msg_w_ctod_t* write_msg;
	msg_for_rw_t* file_info;

	//init basic information, and it just for test now!!
	write_msg = (msg_w_ctod_t* )MSG_COMM_TO_CMD(common_msg);
	source = common_msg->source;
	//tag = common_msg->unique_tag;
	tag = write_msg->unique_tag;
	msg_buff = (void* )malloc(MAX_CMD_MSG_LEN);//may be should use a message queue here
	buff = this->m_data_buffer;
	file_info = (msg_for_rw_t* )malloc(sizeof(msg_for_rw_t));
	file_info->offset = write_msg->offset;
	file_info->count = write_msg->write_len;
	file_info->file = init_vfs_file(this->d_super_block, this->files_buffer,
			this->f_arr_buff, VFS_WRITE);
	if(m_write_handler(source, tag, file_info, buff, msg_buff) == -1)
	{
		free(msg_buff);
		free(file_info);
		return NULL;
	}
	free(msg_buff);
	free(file_info);
	return NULL;
}
static int resolve_msg(common_msg_t* common_msg)
{
	unsigned short operation_code;
	operation_code = common_msg->operation_code;

#ifdef DATASERVER_COMM_DEBUG
			printf("in resolve_msg and the operation code is %d\n", operation_code);
#endif

	switch(operation_code)
	{
	case MSG_READ:
		//invoke a thread to excuse
		d_read(data_server, common_msg);
		break;
	case MSG_WRITE:
		//invoke a thread to excuse
		d_write(data_server, common_msg);
		break;
	default:
		break;
	}
	return -1;
}

void m_resolve(msg_queue_t * msg_queue)
{
	int offset;
	common_msg_t* t_common_msg;

#ifdef DATASERVER_COMM_DEBUG
	printf("In m_resolve function\n");
#endif

	t_common_msg = (common_msg_t* )malloc(sizeof(common_msg_t));
	while(1)
	{
		pthread_rwlock_rdlock(&msg_queue_rw_lock);
		if(!Q_EMPTY(msg_queue->head_pos, msg_queue->tail_pos))
		{
			pthread_rwlock_unlock(&msg_queue_rw_lock);
			pthread_rwlock_wrlock(&msg_queue_rw_lock);
			offset = msg_queue->head_pos;
			msg_queue->head_pos = (msg_queue->head_pos + 1) % D_MSG_BSIZE;
			msg_queue->current_size = msg_queue->current_size - 1;
			memcpy(t_common_msg, msg_queue->msg + offset, sizeof(common_msg_t));
			pthread_rwlock_unlock(&msg_queue_rw_lock);
			resolve_msg(t_common_msg);
		}
		else
		{
			pthread_rwlock_unlock(&msg_queue_rw_lock);
			//阻塞
		}
	}
	free(t_common_msg);
}

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

data_server_t* init_dataserver(total_size_t t_size, int dev_num)
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
	data_server->m_cmd_queue = (msg_queue_t* )malloc(sizeof(msg_queue_t));
	data_server->m_cmd_queue->msg = (common_msg_t* )malloc(sizeof(common_msg_t)
			* D_MSG_BSIZE);
	data_server->m_cmd_queue->current_size = 0;
	data_server->m_cmd_queue->head_pos = 0;
	data_server->m_cmd_queue->tail_pos = 0;

	//init many kinds of locks
	if(pthread_rwlock_init(&msg_queue_rw_lock, NULL) != 0)
			err_sys("init pthread lock wrong");
	return data_server;
	//end of init
}
