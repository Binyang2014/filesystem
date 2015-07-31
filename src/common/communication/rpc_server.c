/*
 * rpc_server.c
 *
 *  Created on: 2015年7月10日
 *      Author: ron
 */

#include <math.h>
#include "rpc_server.h"
#include "../structure_tool/zmalloc.h"
#include "../structure_tool/syn_tool.h"
#include "../structure_tool/log.h"
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

#if defined(RPC_SERVER_DEBUG)
	log_write(LOG_DEBUG, "RPC_SERVER && THREAD POOL START");
#endif

	//TODO multi_thread access server_thread_cancel may read error status
	while(!server->server_thread_cancel) {
		recv_common_msg(server->recv_buff, ANY_SOURCE, CMD_TAG);
#if defined(RPC_SERVER_DEBUG)
		log_write(LOG_DEBUG, "RPC_SERVER PUT MESSAGE");
#endif
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
	server_stop(server);
	destroy_syn_queue(server->request_queue);
	destroy_thread_pool(server->thread_pool);
	zfree(server->op);
	zfree(server->recv_buff);
	zfree(server);
}
