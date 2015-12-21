/*
 * rpc_client.c
 *
 *  Created on: 2015年7月13日
 *      Author: ron
 */
#include <string.h>
#include <stdlib.h>
#include "rpc_client.h"
#include "message.h"
#include "zmalloc.h"
#include "log.h"

/*--------------------Private Declaration---------------------*/
static int execute(rpc_client_t *client, execute_type_t exe_type);
static void set_recv_buff(rpc_client_t* client, void* buff , uint32_t
		recv_buff_len);
static void set_send_buff(rpc_client_t* client, void* buff, uint32_t);
static void set_second_send_buff(rpc_client_t* client, void* buff, uint32_t
		second_send_buff_len);
static void recv_data(rpc_client_t* client, uint32_t len);
static void send_data(rpc_client_t* client, uint32_t len);

/*--------------------Private Implementation------------------*/
static void recv_data(rpc_client_t* client, uint32_t len)
{
	msg_data_t *data_msg;
	void* recv_buff = client->recv_buff;
	uint32_t recv_buff_len = client->recv_buff_len, offset = 0;
	int recv_count, rest_bytes, i;

	//I should be sure this error will not happen
	if(len > recv_buff_len)
	{
		log_write(LOG_ERR, "receive buffer is too small to receive entire message");
		exit(1);
	}
	data_msg = (msg_data_t* )zmalloc(sizeof(msg_data_t));
	recv_count = len / MAX_DATA_CONTENT_LEN;
	rest_bytes = len % MAX_DATA_CONTENT_LEN;
	for(i = 0; i < recv_count; i++)
	{
		recv_data_msg(data_msg, client->target, client->tag, IGNORE_LENGTH);
		memcpy(recv_buff + offset, data_msg->data, data_msg->len);
		offset = offset + data_msg->len;
	}
	if(rest_bytes != 0)
	{
		recv_data_msg(data_msg, client->target, client->tag, rest_bytes);
		memcpy(recv_buff + offset, data_msg->data, data_msg->len);
		offset = offset + data_msg->len;
	}
	zfree(data_msg);
}

static void send_data(rpc_client_t* client, uint32_t len)
{
	msg_data_t* data_msg;
	void* send_buff = client->second_send_buff;
	uint32_t msg_offset = 0, offset = 0, send_buff_len = client->second_send_buff_len;
	int send_count, rest_bytes, i;
	write_c_to_d_t* cmd_msg;

	if(len > send_buff_len)
	{
		log_write(LOG_ERR, "receive buffer is too small to receive entire message");
		exit(1);
	}
	data_msg = (msg_data_t *)zmalloc(sizeof(msg_data_t));
	send_count = len / MAX_DATA_CONTENT_LEN;
	rest_bytes = len % MAX_DATA_CONTENT_LEN;
	cmd_msg = client->send_buff;
	msg_offset = cmd_msg->offset;

	for(i = 0; i < send_count; i++)
	{
		data_msg->len = MAX_DATA_CONTENT_LEN;
		data_msg->offset = msg_offset;
		data_msg->seqno = i;
		data_msg->tail = 0;
		if(rest_bytes == 0 && i == send_count - 1)
		{
			data_msg->tail = 1;
		}
		memcpy(data_msg->data, send_buff + offset, data_msg->len);
		msg_offset = msg_offset + data_msg->len;
		offset = offset + data_msg->len;
		send_data_msg(data_msg, client->target, client->tag, IGNORE_LENGTH);
		msg_offset = msg_offset + data_msg->len;
	}
	if(rest_bytes != 0)
	{
		data_msg->len = rest_bytes;
		data_msg->offset = msg_offset;
		data_msg->seqno = i;
		data_msg->tail = 1;
		memcpy(data_msg->data, send_buff + offset, data_msg->len);
		send_data_msg(data_msg, client->target, client->tag, data_msg->len);
		msg_offset = msg_offset + data_msg->len;
		offset = offset + data_msg->len;
	}
	zfree(data_msg);
}

static int execute(rpc_client_t *client, execute_type_t exe_type)
{
	void* recv_msg_buff = NULL;
	head_msg_t* head_msg = NULL;
	acc_msg_t* acc_msg = NULL;

	if(client->send_buff == NULL)
	{
		log_write(LOG_ERR, "client has not been provided command message");
		return -1;
	}
#if RPC_CLIENT_DEBUG
	log_write(LOG_DEBUG, "RPC CLIENT SEND CMD, buff = %d, target = %d, len = %d, type is %d", 
			client->send_buff, client->target, client->send_buff_len, exe_type);
#endif
	send_cmd_msg(client->send_buff, client->target, client->send_buff_len);
	switch(exe_type)
	{
		case READ_C_TO_D:
			head_msg = zmalloc(sizeof(head_msg_t));
			recv_head_msg(head_msg, client->target, client->tag);
			if(head_msg->op_status != ACC_OK)
			{
				log_write(LOG_ERR, "server do not prepared to send data");
				zfree(head_msg);
				return -1;
			}
			recv_data(client, head_msg->len);
			acc_msg = zmalloc(sizeof(acc_msg_t));
			send_acc_msg(acc_msg, client->target, client->tag, ACC_OK);
#if RPC_CLIENT_DEBUG
			log_write(LOG_INFO, "finish read operation");
#endif
			zfree(head_msg);
			zfree(acc_msg);
			break;

		case WRITE_C_TO_D:
			acc_msg = zmalloc(sizeof(acc_msg_t));
			recv_acc_msg(acc_msg, client->target, client->tag);
			if(acc_msg->op_status != ACC_OK)
			{
				log_write(LOG_ERR, "server, do not prepared to receive data");
				zfree(acc_msg);
				return -1;
			}
			if(client->second_send_buff != NULL && client->second_send_buff_len > 0)
			{
				send_data(client, ((write_c_to_d_t*)client->send_buff)->write_len);
			}
			else
			{
				log_write(LOG_ERR, "client does not offer data yet");
				zfree(acc_msg);
				return -1;
			}
			recv_acc_msg(acc_msg, client->target, client->tag);
			if(acc_msg->op_status != ACC_OK)
			{
				log_write(LOG_ERR, "data server has something wrong and can not save data");
				zfree(acc_msg);
				return -1;
			}
#if RPC_CLIENT_DEBUG
			log_write(LOG_INFO, "finish write operation");
#endif
			zfree(acc_msg);
			break;

		case COMMAND:
			acc_msg = zmalloc(sizeof(acc_msg_t));
			recv_acc_msg(acc_msg, client->target, client->tag);
			if(acc_msg->op_status != ACC_OK)
			{
				log_write(LOG_ERR, "command did not execute successfully");
				zfree(acc_msg);
				return -1;
			}
#if RPC_CLIENT_DEBUG
			log_write(LOG_INFO, "command execute successfully");
#endif
			zfree(acc_msg);
			break;

			//receive buffer will be freed when destroy rpc client or you can
			//free it outside
		case COMMAND_WITH_RETURN:
			head_msg = zmalloc(sizeof(head_msg_t));
			recv_head_msg(head_msg, client->target, client->tag);
#if RPC_CLIENT_DEBUG
			log_write(LOG_DEBUG, "rpc client received head and head length = %d", head_msg->len);
#endif
			if(head_msg->op_status != ACC_OK)
			{
				log_write(LOG_ERR, "server can not deal with this command");
				zfree(head_msg);
				return -1;
			}
			recv_msg_buff = zmalloc(head_msg->len);
			set_recv_buff(client, recv_msg_buff, head_msg->len);
			recv_msg(client->recv_buff, client->target, client->tag, head_msg->len);
#if RPC_CLIENT_DEBUG
			log_write(LOG_DEBUG, "rpc client received ans");
#endif
			zfree(head_msg);
			break;

		case COMMAND_WITHOUT_RETURN:
			break;

		case STOP_SERVER:
			acc_msg = zmalloc(sizeof(acc_msg_t));
			recv_acc_msg(acc_msg, client->target, client->tag);
			if(acc_msg->op_status != ACC_OK)
			{
				log_write(LOG_ERR, "can not stop server now");
				zfree(acc_msg);
				return -1;
			}
#if RPC_CLIENT_DEBUG
			//send this message again to let server stop
			log_write(LOG_INFO, "server will stop soon");
#endif
			zfree(acc_msg);
			break;

		default:
			log_write(LOG_ERR, "offer wrong command");
			return -1;
	}
	return 0;
}

/*---------------------API Implementation----------------------*/
rpc_client_t *create_rpc_client(int client_id, int target, int tag)
{
	rpc_client_t *this = zmalloc(sizeof(rpc_client_t));
	this->op = zmalloc(sizeof(rpc_client_op_t));
	this->client_id = client_id;
	this->target = target;
	this->tag = tag;
	this->send_buff = this->second_send_buff = this->recv_buff = NULL;
	this->recv_buff_len = this->second_send_buff_len = 0;
	if(tag == CMD_TAG)
	{
		log_write(LOG_WARN, "you can not send message use command tag");
		zfree(this->op);
		zfree(this);
		return NULL;
	}
	this->op->execute = execute;
	this->op->set_recv_buff = set_recv_buff;
	this->op->set_send_buff = set_send_buff;
	this->op->set_second_send_buff = set_second_send_buff;
	return this;
}

static void set_recv_buff(struct rpc_client* client, void* buff , uint32_t recv_buff_len)
{
	client->recv_buff = buff;
	client->recv_buff_len = recv_buff_len;
}

static void set_send_buff(struct rpc_client* client, void* buff, uint32_t send_buff_len)
{
	client->send_buff = buff;
	client->send_buff_len = send_buff_len;
}

static void set_second_send_buff(struct rpc_client* client, void* buff, uint32_t second_send_buff_len)
{
	client->second_send_buff = buff;
	client->second_send_buff_len = second_send_buff_len;
}

void destroy_rpc_client(rpc_client_t *client) 
{
	zfree(client->op);
	zfree(client);
}
