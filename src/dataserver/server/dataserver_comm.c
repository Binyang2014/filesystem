/*
 * created on 2015.4.7
 * author: Binyang
 *
 * This file complete functions defined in dastaserver_comm.h
 */
#include <string.h>
#include <mpi.h>
#include "dataserver_comm.h"
#include "../../tool/errinfo.h"
#include "../structure/vfs_structure.h"
#include "../../tool/message.h"

static 	pthread_rwlock_t msg_queue_rw_lock;

void init_msg()
{
	if(pthread_rwlock_init(&msg_queue_rw_lock, NULL) != 0)
		err_sys("init pthread lock wrong");
}

void m_cmd_receive(msg_queue_t * msg_queue)
{
	int offset;
	char* start_pos;
	common_msg_t* t_common_msg;
	MPI_Status status;

	while (1)
	{
		//这里需要用信号量来控制，单纯的解决不了问题，下面的逻辑是有问题的
		//以后再进行改动，在数据节点上相当于单生产者单消费者模型，可以使用读写锁来实现
		//read lock
		if (!Q_FULL(msg_queue->head_pos, msg_queue->tail_pos))
		{
			//read unlock
			//write lock
			offset = msg_queue->tail_pos;
			msg_queue->tail_pos = msg_queue->tail_pos + 1;
			//write unlock
		}
		else
		{
			//Maybe if the space is no big enough, I should realloc if and make it a bigger heap
#ifdef DATASERVER_COMM_DEBUG
			err_msg("The message queue already full!!!");
			return;
#endif
		}

		t_common_msg = msg_queue->msg + offset;
		start_pos = (char*) t_common_msg + COMMON_MSG_HEAD;
		MPI_Recv(start_pos, MAX_CMD_MSG_LEN, MPI_CHAR, MPI_ANY_SOURCE,
				MPI_ANY_TAG,
				MPI_COMM_WORLD, &status);
		t_common_msg->source = status->MPI_SOURCE;
		t_common_msg->unique_tag = status->MPI_TAG;
	}
}

//handle the request from client about reading from a file
int m_read_handler(int source, int tag, msg_for_rw_t* file_info, char* buff)
{
	char* msg_buff;
	int ans = 0, msg_blocks, msg_rest, i, temp_len, temp_ans;
	off_t offset;
	MPI_Status status;

	msg_buff = (char* )malloc(sizeof(char) * MAX_DATA_MSG_LEN);
	if(msg_buff == NULL)
	{
		err_ret("when alloc msg_buff");
		free(msg_buff);
		//send error message to client
		return -1;
	}
	MPI_Recv(msg_buff, MAX_CMD_MSG_LEN, MPI_CHAR, source, tag, MPI_COMM_WORLD, &status);
	if(*(unsigned short*)msg_buff != MSG_ACC)
	{
		err_msg("The client do not ready to receive message now");
		free(msg_buff);
		//send error message to client
		return -1;
	}

	msg_blocks = file_info->count / MAX_DATA_CONTENT_LEN;
	msg_rest = file_info->count % MAX_DATA_CONTENT_LEN;
	offset = file_info->offset;

	for(i = 0; i < msg_blocks - 1; i++)
	{
		//read a block from vfs
		if((temp_ans = vfs_read(file_info->file, buff, MAX_DATA_CONTENT_LEN, offset)) == -1)
		{
			err_msg("Something wrong when read from data server");
			free(msg_buff);
			//send error message to client
			return -1;
		}

		memcpy(((msg_data_t* )msg_buff)->data, buff, MAX_DATA_CONTENT_LEN);
		offset = offset + MAX_DATA_MSG_LEN;
		((msg_data_t* )msg_buff)->len = MAX_DATA_CONTENT_LEN;
		((msg_data_t* )msg_buff)->seqno = i;
		((msg_data_t* )msg_buff)->offset = offset;//I don't know if this operation is right
		((msg_data_t* )msg_buff)->tail = 0;

		MPI_Send(msg_buff, MAX_DATA_MSG_LEN, MPI_CHAR, source, tag, MPI_COMM_WORLD);
		ans = ans + temp_ans;
	}

	//send tail message
	if(msg_rest == 0)
		((msg_data_t* )msg_buff)->len = MAX_DATA_CONTENT_LEN;
	else
		((msg_data_t* )msg_buff)->len = msg_rest;

	temp_len = ((msg_data_t* )msg_buff)->len;
	if((temp_ans = vfs_read(file_info->file, buff, temp_len, offset)) == -1)
	{
		err_msg("Something wrong when read from data server");
		free(msg_buff);
		//send error message to client
		return -1;
	}
	memcpy(((msg_data_t* )msg_buff)->data, buff, temp_len);
	offset = offset + temp_len;
	((msg_data_t* )msg_buff)->seqno = i;
	((msg_data_t* )msg_buff)->offset = offset;//I don't know if this operation is right
	((msg_data_t* )msg_buff)->tail = 1;
	MPI_Send(msg_buff, MAX_DATA_MSG_LEN, MPI_CHAR, source, tag, MPI_COMM_WORLD);
	ans = ans + temp_ans;
	free(msg_buff);

	//maybe there is need to receive Acc from client
	return ans;
}

//handle the request from client about writing to a file
int m_write_handler(int source, int tag, msg_for_rw_t* file_info, char* buff)
{
	char* msg_buff;
	int ans = 0, msg_blocks, msg_rest, i, temp_len, temp_ans;
	off_t offset;
	MPI_Status status;

	msg_buff = (char* )malloc(sizeof(char) * MAX_DATA_MSG_LEN);
	if(msg_buff == NULL)
	{
		err_ret("when alloc msg_buff");
		free(msg_buff);
		//send error message to client
		return -1;
	}
	MPI_Recv(msg_buff, MAX_CMD_MSG_LEN, MPI_CHAR, source, tag, MPI_COMM_WORLD, &status);
	if(*(unsigned short*)msg_buff != MSG_ACC)
	{
		err_msg("The client do not ready to send message now");
		free(msg_buff);
		//send error message to client
		return -1;
	}

	msg_blocks = file_info->count / MAX_DATA_CONTENT_LEN;
	msg_rest = file_info->count % MAX_DATA_CONTENT_LEN;
	offset = file_info->offset;

	for(i = 0; i < msg_blocks - 1; i++)
	{
		//received message from client
		//if we need seq number to keep message in order??
		MPI_Revc(msg_buff, MAX_DATA_MSG_LEN, source, tag, MPI_COMM_WORLD);
		memcpy(buff, ((msg_data_t *)msg_buff)->data, MAX_DATA_CONTENT_LEN);
		//write to file system
		if((temp_ans = vfs_write(file_info->file, buff, MAX_DATA_CONTENT_LEN, offset)) == -1)
		{
			err_msg("Something wrong when read from data server");
			free(msg_buff);
			//send error message to client
			return -1;
		}

		offset = offset + MAX_DATA_MSG_LEN;
		ans = ans + temp_ans;
	}

	//send tail message
	if(msg_rest == 0)
		temp_len = MAX_DATA_CONTENT_LEN;
	else
		temp_len = msg_rest;

	MPI_Revc(msg_buff, MAX_DATA_MSG_LEN, source, tag, MPI_COMM_WORLD);
	memcpy(buff, ((msg_data_t* )msg_buff)->data, temp_len);
	if((temp_ans = vfs_write(file_info->file, buff, temp_len, offset)) == -1)
	{
		err_msg("Something wrong when write to data server");
		free(msg_buff);
		//send error message to client
		return -1;
	}

	offset = offset + temp_len;
	ans = ans + temp_ans;
	free(msg_buff);

	//maybe there is need to send Acc to client
	return ans;
}
