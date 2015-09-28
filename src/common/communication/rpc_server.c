/*
 * rpc_server.c
 *
 *  Created on: 2015年7月10日
 *      Author: ron
 */

#include <math.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include "rpc_server.h"
#include "zmalloc.h"
#include "syn_tool.h"
#include "log.h"
#include "basic_list.h"
#include "message.h"

/*--------------------Private Declaration---------------------*/
static void server_start(rpc_server_t *server);
static void server_start2(rpc_server_t *server);
static void server_stop_send_queue(rpc_server_t *server);
static void server_stop(rpc_server_t *server);
static void server_stop2(rpc_server_t *server);
static int send_result(void *param, int dst, int tag, int len, msg_type_t type);
static void send_to_queue(rpc_server_t *server, void *param, int dst, int tag,
		int len);
static int recv_reply(void* param, int source, int tag, msg_type_t type);
static void* send_msg_from_queue(void* server);

/*--------------------Private Implementation------------------*/
static void server_start(rpc_server_t *server)
{
	server->thread_pool->tp_ops->start(server->thread_pool);

	log_write(LOG_DEBUG, "RPC_SERVER && THREAD POOL START");
	if(server->send_queue != NULL)
		pthread_create(&server->qsend_tid, NULL, send_msg_from_queue, server);

	//TODO multi_thread access server_thread_cancel may read error status
	while(!server->server_thread_cancel) {
		recv_common_msg(server->recv_buff, ANY_SOURCE, CMD_TAG);
		server->request_queue->op->syn_queue_push(server->request_queue, server->recv_buff);
	}
	//no more message need to send
	server_stop_send_queue(server);
}

static void server_start2(rpc_server_t *server)
{
	server->thread_pool->tp_ops->start(server->thread_pool);

	log_write(LOG_DEBUG, "RPC_SERVER && THREAD POOL START");
	if(server->send_queue != NULL)
		pthread_create(&server->qsend_tid, NULL, send_msg_from_queue, server);
}

static void server_stop(rpc_server_t *server) {
	server->server_thread_cancel = 1;
}

static void server_stop_send_queue(rpc_server_t *server)
{
	if(server->send_queue != NULL)
	{
		while(!server->send_queue->queue->basic_queue_op->is_empty(server->send_queue->queue))
			usleep(50);
		pthread_cancel(server->qsend_tid);
		pthread_join(server->qsend_tid, NULL);
		pthread_mutex_unlock(server->send_queue->queue_mutex);
		log_write(LOG_DEBUG, "send queue thread has been canceled");
	}
}

static void server_stop2(rpc_server_t *server)
{
	server_stop_send_queue(server);
	server->server_commit_cancel = 1;
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

static void send_to_queue(rpc_server_t *server, void *param, int dst, int tag,
		int len)
{
	syn_queue_t *send_queue = server->send_queue;
	rpc_send_msg_t *rpc_send_msg = server->send_buff;
	assert(rpc_send_msg->msg == NULL);

	rpc_send_msg->dst = dst;
	rpc_send_msg->tag = tag;
	rpc_send_msg->length = len;
	rpc_send_msg->msg = zmalloc(len);
	memcpy(rpc_send_msg->msg, param, len);
	send_queue->op->syn_queue_push(send_queue, rpc_send_msg);
	rpc_send_msg->msg = NULL;
}

static void clean_up(void *rpc_send_msg)
{
	zfree(rpc_send_msg);
}

static void* send_msg_from_queue(void* server)
{
	rpc_server_t *rpc_server = (rpc_server_t *)server;
	syn_queue_t *send_queue = rpc_server->send_queue;
	rpc_send_msg_t *rpc_send_msg = zmalloc(sizeof(rpc_send_msg_t));

	pthread_cleanup_push(clean_up, rpc_send_msg);
	while(1)
	{
		send_queue->op->syn_queue_pop(send_queue, rpc_send_msg);
		send_msg(rpc_send_msg->msg, rpc_send_msg->dst, rpc_send_msg->tag,
				rpc_send_msg->length);
		zfree(rpc_send_msg->msg);
		rpc_send_msg->msg = NULL;
	}

	pthread_cleanup_pop(0);
	return NULL;
}

static int recv_reply(void* param, int source, int tag, msg_type_t type)
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
	this->send_queue = NULL;
	this->send_buff = NULL;
	this->server_id = server_id;
	this->server_thread_cancel = 0;
	this->server_commit_cancel = 0;
	this->thread_pool = alloc_thread_pool(thread_num, this->request_queue, resolve_handler);

	this->op = zmalloc(sizeof(rpc_server_op_t));
	this->op->server_start = server_start;
	this->op->server_start2 = server_start2;
	this->op->server_stop = server_stop;
	this->op->server_stop2 = server_stop2;
	this->op->send_result = send_result;
	this->op->send_to_queue = send_to_queue;
	this->op->recv_reply = recv_reply;

	this->recv_buff = zmalloc(sizeof(common_msg_t));
	return this;
}

rpc_server_t *create_rpc_server2(int thread_num, int recv_qsize, int send_qsize,
		int server_id, resolve_handler_t resolve_handler)
{
	rpc_server_t *this = create_rpc_server(thread_num, recv_qsize, server_id,
			resolve_handler);
	this->send_queue = alloc_syn_queue(send_qsize, sizeof(rpc_send_msg_t));
	this->send_buff = zmalloc(sizeof(rpc_send_msg_t));
	this->send_buff->msg = NULL;

	return this;
}

void destroy_rpc_server(rpc_server_t *server)
{
	while(server->server_commit_cancel != 1);
	//wait client receive ack message
	usleep(500);
	destroy_thread_pool(server->thread_pool);
	destroy_syn_queue(server->request_queue);
	if(server->send_queue != NULL)
		destroy_syn_queue(server->send_queue);
	zfree(server->op);
	zfree(server->recv_buff);
	zfree(server->send_buff);
	zfree(server);
}

/*--------------------TO STOP SERVER---------------------*/
int init_server_stop_handler(event_handler_t *event_handler, void* server,
		void* common_msg)
{
	list_t *list;
	void *cmd_msg;

	event_handler->special_struct = server;
	event_handler->event_buffer_list = list_create();
	if( (list = event_handler->event_buffer_list) == NULL )
	{
		log_write(LOG_ERR, "error when allocate list");
		return -1;
	}
	list_set_free_method(list, zfree);
	cmd_msg = zmalloc(MAX_CMD_MSG_LEN);
	memcpy(cmd_msg, MSG_COMM_TO_CMD(common_msg), MAX_CMD_MSG_LEN);
	list->list_ops->list_add_node_head(list, cmd_msg);
	return 0;
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
