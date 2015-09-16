/*
 * created on 2015.4.7
 * modified on 2015.7.29
 * author: Binyang
 *
 * This file complete functions defined in dastaserver_comm.h
 */
#include <string.h>
#include <stdlib.h>
#include "dataserver_handler.h"
#include "dataserver_buff.h"
#include "message.h"
#include "log.h"
#include "vfs_structure.h"
#include "threadpool.h"
#include "zmalloc.h"

/*=========================INTERNEL FUNCITION EVOKED BY HANDLER================================*/
static int read_from_vfs(dataserver_file_t *file, msg_data_t* buff, size_t count,
		off_t offset, int seqno, int tail)
{
	int ans;
	char* data_buff = (char*)buff->data;

	//read a block from vfs
	if((ans = vfs_read(file, data_buff, count, offset)) == -1)
	{
		err_msg("Something wrong when read from data server");
		return -1;
	}

	buff->len = ans;
	buff->seqno = seqno;
	buff->offset = offset + ans;//I don't know if this operation is right

	//read to the end of the file unexpectedly
	if(ans < count)
		buff->tail = 1;
	buff->tail = tail;
	return ans;
}

/**
 * handle the request from client about reading from a file
 * buff should be allocated properly outside
 */
static int m_read_handler(data_server_t* data_server, int source, int tag, msg_for_rw_t* file_info,
		void* buff)
{
	int ans = 0, msg_blocks, msg_rest, i, temp_ans = 0;
	off_t offset;

#ifdef DATASERVER_COMM_DEBUG
	log_write(LOG_DEBUG, "start read from data server");
#endif

	//caculate many many data messages should be sent
	msg_blocks = file_info->count / MAX_DATA_CONTENT_LEN;
	msg_rest = file_info->count % MAX_DATA_CONTENT_LEN;
	offset = file_info->offset;
	msg_blocks = msg_blocks + (msg_rest ? 1 : 0);

	for(i = 0; i < msg_blocks - 1; i++)
	{
		if( (temp_ans = read_from_vfs(file_info->file, buff, MAX_DATA_CONTENT_LEN,
				offset, i, 0)) == -1 )
			return -1;

		//send message to client
		data_server->rpc_server->op->send_result(buff, source, tag,
				temp_ans, DATA);

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
						offset, i, 1)) == -1 )
			return -1;
	}
	else
	{	if( (temp_ans = read_from_vfs(file_info->file, buff, msg_rest,
						offset, i, 1)) == -1 )
			return -1;
	}

	data_server->rpc_server->op->send_result(buff, source, tag,
			temp_ans, DATA);
	ans = ans + temp_ans;

#ifdef DATASERVER_COMM_DEBUG
	log_write(LOG_DEBUG, "read finished and the bytes of read is %d", ans);
#endif

	//maybe there is need to receive Acc from client
	return ans;
}

//handle the request from client about writing to a file
static int write_to_vfs(dataserver_file_t *file, msg_data_t* buff, off_t offset)
{
	int ans, len;
	char* data_buff;
	len = buff->len;
	data_buff = (char*)buff->data;

	//write to file system messages are in order
	if((ans = vfs_write(file, data_buff, len, offset)) == -1)
	{
		err_msg("Something wrong when read from data server");
		return -1;
	}
	return ans;
}

//write message handler
static int m_write_handler(data_server_t* data_server, int source, int tag, msg_for_rw_t* file_info,
		void* buff)
{
	int ans = 0, msg_blocks, msg_rest, i, temp_ans = 0;
	off_t offset;

	msg_blocks = file_info->count / MAX_DATA_CONTENT_LEN;
	msg_rest = file_info->count % MAX_DATA_CONTENT_LEN;
	offset = file_info->offset;
	msg_blocks = msg_blocks + (msg_rest ? 1 : 0);

#ifdef DATASERVER_COMM_DEBUG
	log_write(LOG_DEBUG, "will receive message from client");
#endif

	for(i = 0; i < msg_blocks; i++)
	{
		//received message from client, all message will be ordered
		data_server->rpc_server->op->recv_reply(buff, source, tag, DATA);

		if((temp_ans = write_to_vfs(file_info->file, buff, offset)) == -1)
			return -1;

		offset = offset + temp_ans;
		ans = ans + temp_ans;
	}

#ifdef DATASERVER_COMM_DEBUG
	log_write(LOG_DEBUG, "write finish and the number of write bytes is %d", ans);
#endif

	//maybe there is need to send Acc to client
	return ans;
}

/*====================================REAL HANDLERS===========================================*/

static void resolve_rw_handler_buffer(event_handler_t* event_handle,
		rw_handle_buff_t* handle_buff)
{
	list_node_t* node;
	list_t* buffer_list = event_handle->event_buffer_list;
	list_iter_t* iter = buffer_list->list_ops->list_get_iterator(buffer_list, 0);

	node = buffer_list->list_ops->list_next(iter);
	handle_buff->common_msg = list_node_value(node);

	node = buffer_list->list_ops->list_next(iter);
	handle_buff->data_buffer = list_node_value(node);

	node = buffer_list->list_ops->list_next(iter);
	handle_buff->file_info->file = list_node_value(node);

	node = buffer_list->list_ops->list_next(iter);
	handle_buff->f_arr_buff =  list_node_value(node);

	buffer_list->list_ops->list_release_iterator(iter);
}

static void release_rw_handler_buffer(event_handler_t* event_handle)
{
	list_node_t* node;
	list_t* buffer_list = event_handle->event_buffer_list;
	list_iter_t* iter = buffer_list->list_ops->list_get_iterator(buffer_list, 0);
	data_server_t* dataserver = event_handle->special_struct;
	void* value;

	node = buffer_list->list_ops->list_next(iter);
	value = list_node_value(node);
	reture_common_msg_buff(dataserver, value);

	node = buffer_list->list_ops->list_next(iter);
	value = list_node_value(node);
	return_data_buff(dataserver, value);

	node = buffer_list->list_ops->list_next(iter);
	value = list_node_value(node);
	return_file_info_buff(dataserver, value);

	node = buffer_list->list_ops->list_next(iter);
	value = list_node_value(node);
	return_f_arr_buff(dataserver, value);

	buffer_list->list_ops->list_release_iterator(iter);
	return_buffer_list(dataserver, buffer_list);
}

void d_read_handler(event_handler_t* event_handle)
{
	int source, tag, i;
	rw_handle_buff_t handle_buff;
	read_c_to_d_t* read_msg = NULL;
	data_server_t* this = NULL;
	head_msg_t* head_msg = NULL;
	acc_msg_t* acc_msg = NULL;

	//allocate file_info this structure do not need buffer
	handle_buff.file_info = (msg_for_rw_t*)zmalloc(sizeof(msg_for_rw_t));
	resolve_rw_handler_buffer(event_handle, &handle_buff);
	//Data server should be the special structure. It is not a buffer
	this = event_handle->special_struct;

	//initial basic information from handle_buffer and event_handle
	read_msg = (read_c_to_d_t* )MSG_COMM_TO_CMD(handle_buff.common_msg);
	source = handle_buff.common_msg->source;
	tag = read_msg->unique_tag;

	//initial read operation
	handle_buff.file_info->offset = read_msg->offset;
	handle_buff.file_info->count = read_msg->read_len;
	handle_buff.f_arr_buff->hash_table_size = read_msg->chunks_count;
	for(i = 0; i < read_msg->chunks_count; i++)
		handle_buff.f_arr_buff->chunks_arr[i] = read_msg->chunks_id_arr[i];
	handle_buff.file_info->file = init_vfs_file(this->d_super_block, handle_buff.file_info->file,
			handle_buff.f_arr_buff, VFS_READ);

	//send head message to client
	head_msg = (head_msg_t*)zmalloc(sizeof(head_msg_t));
	if(handle_buff.file_info->file == NULL)
	{
		head_msg->len = IGNORE_LENGTH;
		head_msg->op_status = READ_FAIL;
		this->rpc_server->op->send_result(head_msg, source, tag, IGNORE_LENGTH, HEAD);
		zfree(head_msg);
		release_rw_handler_buffer(event_handle);
		zfree(handle_buff.file_info);
		return;
	}
	head_msg->len = read_msg->read_len;
	head_msg->op_status = ACC_OK;
	this->rpc_server->op->send_result(head_msg, source, tag, IGNORE_LENGTH, HEAD);
	zfree(head_msg);

	if(m_read_handler(this, source, tag, handle_buff.file_info,
				handle_buff.data_buffer) == -1)
	{
		log_write(LOG_ERR, "internal wrong when reading data and data server is crushed");
		release_rw_handler_buffer(event_handle);
		zfree(handle_buff.file_info);
		exit(1);
	}

	//receive accept message
	acc_msg = zmalloc(sizeof(acc_msg_t));
	this->rpc_server->op->recv_reply(acc_msg, source, tag, ACC);
	if(acc_msg->op_status != ACC_OK)
		log_write(LOG_ERR, "read did not success");
	zfree(acc_msg);
	release_rw_handler_buffer(event_handle);
	zfree(handle_buff.file_info);
}

void d_write_handler(event_handler_t* event_handle)
{
	int source, tag, i;
	write_c_to_d_t* write_msg = NULL;;
	data_server_t* this = NULL;;
	rw_handle_buff_t handle_buff;
	acc_msg_t* acc_msg = NULL;

	//allocate file_info this structure do not need buffer
	handle_buff.file_info = (msg_for_rw_t*)zmalloc(sizeof(msg_for_rw_t));
	resolve_rw_handler_buffer(event_handle, &handle_buff);
	//Data server should be the special structure. It is not a buffer
	this = event_handle->special_struct;

	//init basic information, and it just for test now!!
	write_msg = (write_c_to_d_t* )MSG_COMM_TO_CMD(handle_buff.common_msg);
	source = handle_buff.common_msg->source;
	tag = write_msg->unique_tag;

	handle_buff.file_info->offset = write_msg->offset;
	handle_buff.file_info->count = write_msg->write_len;
	handle_buff.f_arr_buff->hash_table_size = write_msg->chunks_count;
	for(i = 0; i < write_msg->chunks_count; i++)
		handle_buff.f_arr_buff->chunks_arr[i] = write_msg->chunks_id_arr[i];
	handle_buff.file_info->file = init_vfs_file(this->d_super_block, handle_buff.file_info->file,
			handle_buff.f_arr_buff, VFS_WRITE);

	//send accept message to client
	acc_msg = (acc_msg_t*)zmalloc(sizeof(acc_msg_t));
	if(handle_buff.file_info == NULL)
	{
		acc_msg->op_status = INIT_WRITE_FAIL;
		this->rpc_server->op->send_result(acc_msg, source, tag, IGNORE_LENGTH,
				ACC);
		zfree(acc_msg);
		release_rw_handler_buffer(event_handle);
		zfree(handle_buff.file_info);
		return;
	}
	acc_msg->op_status = ACC_OK;
	this->rpc_server->op->send_result(acc_msg, source, tag, IGNORE_LENGTH, ACC);

	//recv data and send acc to client
	if(m_write_handler(this, source, tag, handle_buff.file_info,
				handle_buff.data_buffer) == -1)
	{
		//do somthing here
		acc_msg->op_status = WRITE_FAIL;
		log_write(LOG_ERR, "wrong when write data to data server");
		this->rpc_server->op->send_result(acc_msg, source, tag, IGNORE_LENGTH,
				ACC);
		zfree(acc_msg);
		release_rw_handler_buffer(event_handle);
		zfree(handle_buff.file_info);
		return;
	}
	acc_msg->op_status = ACC_OK;
	this->rpc_server->op->send_result(acc_msg, source, tag, IGNORE_LENGTH, ACC);

	zfree(acc_msg);
	release_rw_handler_buffer(event_handle);
	zfree(handle_buff.file_info);
}
