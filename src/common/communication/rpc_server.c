/*
 * rpc_server.c
 *
 *  Created on: 2015年7月10日
 *      Author: ron
 */

#include <math.h>
#include <unistd.h>
#include "rpc_server.h"
#include "../structure_tool/zmalloc.h"
#include "../structure_tool/syn_tool.h"
#include "../structure_tool/log.h"
#include "../structure_tool/basic_list.h"
#include "message.h"

/*--------------------Private Declaration---------------------*/
static void server_start(rpc_server_t *server);
static void server_stop(rpc_server_t *server);
static int send_result(void *param, int dst, int tag, int len, msg_type_t type);
static int recv_reply(void* param, int source, int tag, msg_type_t type);

/*--------------------Private Implementation------------------*/
static void server_start(rpc_server_t *server)
{
	server->thread_pool->tp_ops->start(server->thread_pool);

	log_write(LOG_DEBUG, "RPC_SERVER && THREAD POOL START");

	//TODO multi_thread access server_thread_cancel may read error status
	while(!server->server_thread_cancel) {
		recv_common_msg(server->recv_buff, ANY_SOURCE, CMD_TAG);

		log_write(LOG_DEBUG, "RPC_SERVER PUT MESSAGE");

		server->request_queue->op->syn_queue_push(server->request_queue, server->recv_buff);
	}
}

static void server_stop(rpc_server_t *server) {
	server->server_thread_cancel = 1;
}

//only data message and ans message need len
static int send_result(void *param, int dst, int tag, int len, 
		msg_type_t type)
{
	head_msg_t head_msg;

	switch(type)
	{
		case ACC:
			send_acc_msg(param, dst, tag, ACC_IGNORE);
			break;
		case DATA:
			send_data_msg(param, dst, tag, len);
			break;
		case ANS:
			head_msg.op_status = ACC_OK;
			head_msg.len = len;

			send_head_msg(&head_msg, dst, tag);
			send_msg(param, dst, tag, head_msg.len);
			break;
		case CMD:
			send_cmd_msg(param, dst, tag);
			break;
		case HEAD:
			send_head_msg(param, dst, tag);
			break;
		default:
			log_write(LOG_ERR, "wrong type of message, check your code");
			return -1;
	}
	return 0;
}

int recv_reply(void* param, int source, int tag, msg_type_t type)
{
	switch(type)
	{
		case ACC:
			recv_acc_msg(param, source, tag);
			break;
		case DATA:
			recv_data_msg(param, source, tag, IGNORE_LENGTH);
			break;
		default:
			log_write(LOG_ERR, "wrong type of message, check your code");
			return -1;
	}
	return 0;
}
/*--------------------API Implementation--------------------*/
rpc_server_t *create_rpc_server(int thread_num, int queue_size,
		int server_id, resolve_handler_t resolve_handler)
{
	rpc_server_t *this = zmalloc(sizeof(rpc_server_t));

	this->request_queue = alloc_syn_queue(queue_size, sizeof(common_msg_t));
	this->server_id = server_id;
	this->server_thread_cancel = 0;
	this->server_commit_cancel = 0;
	this->thread_pool = alloc_thread_pool(thread_num, this->request_queue, resolve_handler);

	this->op = zmalloc(sizeof(rpc_server_op_t));
	this->op->server_start = server_start;
	this->op->server_stop = server_stop;
	this->op->send_result = send_result;
	this->op->recv_reply = recv_reply;

	this->recv_buff = zmalloc(sizeof(common_msg_t));
	return this;
}

void destroy_rpc_server(rpc_server_t *server)
{
	while(server->server_commit_cancel != 1);
	//wait client receive ack message
	sleep(1);
	destroy_syn_queue(server->request_queue);
	destroy_thread_pool(server->thread_pool);
	zfree(server->op);
	zfree(server->recv_buff);
	zfree(server);
}

/*--------------------TO STOP SERVER---------------------*/
void init_server_stop_handler(event_handler_t *event_handler, void* server,
		void* common_msg)
{
	list_t *list;

	event_handler->special_struct = server;
	event_handler->event_buffer_list = list_create();
	list = event_handler->event_buffer_list;
	list->list_ops->list_add_node_head(list,
			MSG_COMM_TO_CMD(common_msg));
}

void server_stop_handler(event_handler_t *event_handler)
{
	rpc_server_t *server = event_handler->special_struct;
	list_t *list = event_handler->event_buffer_list;
	stop_server_msg_t *stop_server_msg = list_node_value(list->list_ops->list_index(list,
				0));
	acc_msg_t *acc_msg = (acc_msg_t*)zmalloc(sizeof(acc_msg_t));

	log_write(LOG_INFO, "This server wiil be stoped soon!!!");
	if(server->server_thread_cancel == 0)
		server->op->server_stop(server);
	//mark it, and will tell destroy functoin later
	else
		server->server_commit_cancel = -1;

	acc_msg->op_status = ACC_OK;
	server->op->send_result(acc_msg, stop_server_msg->source, stop_server_msg->tag, IGNORE_LENGTH, ACC);
	zfree(acc_msg);
	list_release(list);
	event_handler->event_buffer_list = NULL;
	//tell destory function to destroy this server
	if(server->server_commit_cancel == -1)
		server->server_commit_cancel = 1;
}
