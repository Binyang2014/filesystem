/*
 * rpc_client.c
 *
 *  Created on: 2015年7月13日
 *      Author: ron
 */
#include "rpc_client.h"
#include "message.h"
#include "../structure_tool/zmalloc.h"
#include "../structure_tool/log.h"

/*--------------------Private Declaration---------------------*/
static int execute(rpc_client_t *client, execute_type_t exe_type);
static void set_recv_buff(rpc_client_t* client, void* buff , uint32_t
		recv_buff_len);
static void set_send_buff(rpc_client_t* client, void* buff);
static void set_second_send_buff(rpc_client_t* client, void* buff, uint32_t
		second_send_buff_len);

/*--------------------Private Implementation------------------*/
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
	send_cmd_msg(client->send_buff, client->target, CMD_TAG);
	switch(exe_type)
	{
		case READ:
			head_msg = zmalloc(sizeof(head_msg_t));
			recv_head_msg(head_msg, client->target, client->tag);
			if(head_msg->op_status != ACC_OK)
			{
				log_write(LOG_ERR, "server do not prepared to send data");
				zfree(head_msg);
				return -1;
			}
			recv_data_msg(client->recv_buff, client->target, client->tag,
					head_msg->len);
			acc_msg = zmalloc(sizeof(acc_msg_t));
			send_acc_msg(acc_msg, client->target, client->tag, ACC_OK);
			log_write(LOG_INFO, "finish read operation");
			zfree(head_msg);
			zfree(acc_msg);
			break;

		case WRITE:
			acc_msg = zmalloc(sizeof(acc_msg_t));
			recv_acc_msg(acc_msg, client->target, client->tag);
			if(acc_msg->op_status != ACC_OK)
			{
				log_write(LOG_ERR, "server, do not prepared to receive data");
				zfree(acc_msg);
				return -1;
			}
			if(client->second_send_buff != NULL &&
					client->second_send_buff_len > 0)
				send_data_msg(client->second_send_buff, client->target, client->tag,
						client->second_send_buff_len);
			else
			{
				log_write(LOG_ERR, "client does not offer data yet");
				zfree(acc_msg);
				return -1;
			}
			recv_acc_msg(acc_msg, client->target, client->tag);
			if(acc_msg->op_status != ACC_OK)
			{
				log_write(LOG_ERR, "data server has something wrong and can not\
						save data");
				zfree(acc_msg);
				return -1;
			}
			log_write(LOG_INFO, "finish write operation");
			zfree(acc_msg);
			break;

		case COMMAND:
			acc_msg = zmalloc(sizeof(acc_msg_t));
			recv_acc_msg(acc_msg, client->target, client->tag);
			if(acc_msg->op_status != ACC_OK)
			{
				log_write(LOG_ERR, "command did not execute seccessfully");
				zfree(acc_msg);
				return -1;
			}
			log_write(LOG_INFO, "command execute successfully");
			break;

			//receive buffer will be freed when destroy rpc client or you can
			//free it outside
		case COMMAND_WITH_RETURN:
			head_msg = zmalloc(sizeof(head_msg_t));
			recv_head_msg(head_msg, client->target, client->tag);
			if(head_msg->op_status != ACC_OK)
			{
				log_write(LOG_ERR, "server can not deal with this command");
				zfree(head_msg);
				return -1;
			}
			recv_msg_buff = zmalloc(head_msg->len);
			set_recv_buff(client, recv_msg_buff, head_msg->len);
			recv_msg(client->recv_buff, client->target, client->tag,
					head_msg->len);
			zfree(head_msg);
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
static void set_recv_buff(struct rpc_client* client, void* buff , uint32_t
		recv_buff_len)
{
	client->recv_buff = buff;
	client->recv_buff_len = recv_buff_len;
}

static void set_send_buff(struct rpc_client* client, void* buff)
{
	client->send_buff = buff;
}

static void set_second_send_buff(struct rpc_client* client, void* buff, uint32_t
		second_send_buff_len)
{
	client->second_send_buff = buff;
	client->second_send_buff_len = second_send_buff_len;
}

void destroy_rpc_client(rpc_client_t *client) {
	zfree(client->op);
	zfree(client);
}
