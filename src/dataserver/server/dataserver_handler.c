/*
 * created on 2015.4.7
 * author: Binyang
 *
 * This file complete functions defined in dastaserver_comm.h
 */
#include <string.h>
#include <mpi.h>
#include "dataserver_handler.h"
#include "../../tool/errinfo.h"
#include "../structure/vfs_structure.h"
#include "../../tool/message.h"
#include "../../tool/threadpool.h"

/*=========================INTERNEL FUNCITION EVOKED BY HANDLER================================*/
static int read_from_vfs(dataserver_file_t *file, char* buff, size_t count,
		off_t offset, int seqno, int tail, msg_data_t* msg_buff)
{
	int ans;

	//read a block from vfs
	if((ans = vfs_read(file, buff, count, offset)) == -1)
	{
		err_msg("Something wrong when read from data server");
		return -1;
	}

	memcpy(msg_buff->data, buff, MAX_DATA_CONTENT_LEN);
	msg_buff->len = count;
	msg_buff->seqno = seqno;
	msg_buff->offset = offset + ans;//I don't know if this operation is right

	//read to the end of the file unexpectedly
	if(ans < count)
		msg_buff->tail = 1;
	msg_buff->tail = tail;
	return ans;
}

/**
 * handle the request from client about reading from a file
 * buff and msg_buff should be allocated properly outside
 */
 static int m_read_handler(int source, int tag, msg_for_rw_t* file_info, char* buff,
		 void* msg_buff)
{
	int ans = 0, msg_blocks, msg_rest, i, temp_ans = 0;
	off_t offset;
	mpi_status_t status;

#ifdef DATASERVER_COMM_DEBUG
	printf("start read from data server\n");
#endif

	d_mpi_acc_recv(msg_buff, source, tag, &status);
	//client do not want to receive message, and server will not provide
	if(*(unsigned short*)msg_buff != MSG_ACC)
	{
		err_msg("The client do not ready to receive message now");
		return -1;
	}

	msg_blocks = file_info->count / MAX_DATA_CONTENT_LEN;
	msg_rest = file_info->count % MAX_DATA_CONTENT_LEN;
	offset = file_info->offset;
	msg_blocks = msg_blocks + (msg_rest ? 1 : 0);

#ifdef DATASERVER_COMM_DEBUG
	printf("The acc in read handler\n");
	printf_msg_status(&status);
	printf("send message to client\n");
#endif

	for(i = 0; i < msg_blocks - 1; i++)
	{
		if( (temp_ans = read_from_vfs(file_info->file, buff, MAX_DATA_CONTENT_LEN,
				offset, i, 0, msg_buff)) == -1 )
		{
			//here thread should send error message to client
			return -1;
		}

		//send message to client
		d_mpi_data_send(msg_buff, source, tag);

		//already read at the end of the file
		if(temp_ans == 0)
			return ans;

		offset = offset + temp_ans;
		ans = ans + temp_ans;
	}

	//send tail message
	if(msg_rest == 0)
	{
		if( (temp_ans = read_from_vfs(file_info->file, buff, MAX_DATA_CONTENT_LEN,
						offset, i, 1, msg_buff)) == -1 )
			return -1;
	}
	else
	{	if( (temp_ans = read_from_vfs(file_info->file, buff, msg_rest,
						offset, i, 1, msg_buff)) == -1 )
			return -1;
	}

	d_mpi_data_send(msg_buff, source, tag);
	ans = ans + temp_ans;

#ifdef DATASERVER_COMM_DEBUG
	printf("read finished and the bytes of read is %d\n", ans);
#endif

	//maybe there is need to receive Acc from client
	return ans;
}

//handle the request from client about writing to a file
static int write_to_vfs(dataserver_file_t *file, char* buff, off_t offset,
		msg_data_t* msg_buff)
{
	int ans, len;
	len = msg_buff->len;

	//if we need seq number to keep message in order??
	memcpy(buff, ((msg_data_t *)msg_buff)->data, len);
	//write to file system
	if((ans = vfs_write(file, buff, len, offset)) == -1)
	{
		err_msg("Something wrong when read from data server");
		return -1;
	}
	return ans;
}

//write message handler
static int m_write_handler(int source, int tag, msg_for_rw_t* file_info, char* buff,
		void* msg_buff)
{
	int ans = 0, msg_blocks, msg_rest, i, temp_ans = 0;
	off_t offset;
	mpi_status_t status;

	//waiting for message
	d_mpi_acc_recv(msg_buff, source, tag, &status);

#ifdef DATASERVER_COMM_DEBUG
	printf("accept message in write handler\n");
	printf_msg_status(&status);
#endif

	if(*(unsigned short*)msg_buff != MSG_ACC)
	{
		err_msg("The client do not ready to send message now");
		//send error message to client
		return -1;
	}

	msg_blocks = file_info->count / MAX_DATA_CONTENT_LEN;
	msg_rest = file_info->count % MAX_DATA_CONTENT_LEN;
	offset = file_info->offset;
	msg_blocks = msg_blocks + (msg_rest ? 1 : 0);

#ifdef DATASERVER_COMM_DEBUG
	printf("will receive message from client\n");
#endif

	for(i = 0; i < msg_blocks; i++)
	{
		//received message from client
		//if we need seq number to keep message in order??
		d_mpi_data_recv(msg_buff, source, tag, &status);

		if((temp_ans = write_to_vfs(file_info->file, buff, offset, msg_buff)) == -1)
			return -1;

		offset = offset + temp_ans;
		ans = ans + temp_ans;
	}

#ifdef DATASERVER_COMM_DEBUG
	printf("write finish and the number of write bytes is %d\n", ans);
	printf_msg_status(&status);
#endif

	//maybe there is need to send Acc to client
	return ans;
}

/*====================================REAL HANDLERS===========================================*/

static void resolve_rw_handler_buffer(event_handler_t* event_handle,
		rw_handle_buff_t* handle_buff)
{
//	buffer_t* buffer = event_handle->event_buffer;
//
//	handle_buff->common_msg = buffer;
//	buffer = buffer->next;
//
//	handle_buff->msg_buffer = buffer;
//	buffer = buffer->next;
//
//	handle_buff->data_buffer =buffer;
//	buffer = buffer->next;
//
//	handle_buff->file_info = buffer;
//	buffer = buffer->next;
//
//	handle_buff->f_arr_buff = buffer;
//	buffer = buffer->next;
}

void d_read_handler(event_handler_t* event_handle)
{
	int source, tag;
	rw_handle_buff_t handle_buff;
	msg_r_ctod_t* read_msg;
	data_server_t* this;

	resolve_rw_handler_buffer(event_handle, &handle_buff);
	//Data server should be the special structure. It is not a buffer
	this = event_handle->spcical_struct;

	//initial basic information from handle_buffer and event_handle
	read_msg = (msg_r_ctod_t* )MSG_COMM_TO_CMD(handle_buff->common_msg);
	source = handle_buff->common_msg->source;
	tag = read_msg->unique_tag;

	handle_buff->file_info->offset = read_msg->offset;
	handle_buff->file_info->count = read_msg->read_len;
	handle_buff->file_info->file = init_vfs_file(this->d_super_block, handle_buff->file_info->file,
			handle_buff->f_arr_buff, VFS_READ);

	if(m_read_handler(source, tag, handle_buff->file_info, handle_buff->data_buffer,
			handle_buff->msg_buffer) == -1)
	{
		//do something like sending error message to client
	}

	printf("It's OK here\n");
}

void d_write_handler(event_handler_t* event_handle)
{
	int source, tag;
	msg_w_ctod_t* write_msg;
	data_server_t* this;
	rw_handle_buff_t handle_buff;

	resolve_rw_handler_buffer(event_handle, &handle_buff);
	//Data server should be the special structure. It is not a buffer
	this = event_handle->spcical_struct;

	//init basic information, and it just for test now!!
	write_msg = (msg_w_ctod_t* )MSG_COMM_TO_CMD(handle_buff->common_msg);
	source = handle_buff->common_msg->source;
	//tag = common_msg->unique_tag;
	tag = write_msg->unique_tag;

	handle_buff->file_info->offset = write_msg->offset;
	handle_buff->file_info->count = write_msg->write_len;
	handle_buff->file_info->file = init_vfs_file(this->d_super_block, handle_buff->file_info->file,
			handle_buff->f_arr_buff, VFS_WRITE);

	if(m_write_handler(source, tag, handle_buff->file_info, handle_buff->data_buffer,
			handle_buff->msg_buffer) == -1)
	{
		//do somthing here
	}
}
