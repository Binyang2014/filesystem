/*
 * mpi_rpc.c
 *
 *  Created on: 2015年7月10日
 *      Author: ron
 */

#include <math.h>
#include "mpi_rpc_server.h"
#include "../structure_tool/zmalloc.h"
#include "../structure_tool/syn_tool.h"
#include "../structure_tool/log.h"
#include "message.h"

//#define RPC_SERVER_DEBUG 1

/*--------------------Private Declaration---------------------*/
static void server_start(mpi_rpc_server_t *server);
static void server_end(mpi_rpc_server_t *server);
static void send_result(void *param, int source, int tag, int len);

/*--------------------Global Declaration----------------------*/
mpi_rpc_server_t *create_mpi_rpc_server(int thread_num, int rank, void *(*resolve_handler)(event_handler_t *event_handler, void* msg_queue));
void destroy_mpi_rpc_server(mpi_rpc_server_t *server);

/*--------------------Private Implementation------------------*/
static void server_start(mpi_rpc_server_t *server) {
	server->thread_pool->tp_ops->start(server->thread_pool);

#if defined(RPC_SERVER_DEBUG)
	puts("RPC_SERVER && THREAD POOL START");
#endif

	//TODO multi_thread access server_thread_cancel may read error status
	while(!server->server_thread_cancel) {
		//TODO if multi_client send request to this, Is there any parallel problem ?
		recv_common_msg(server->recv_buff, MPI_ANY_SOURCE, MPI_ANY_TAG);

#if defined(RPC_SERVER_DEBUG)
		puts("RPC_SERVER PUT MESSAGE");
#endif

		server->request_queue->op->syn_queue_push(server->request_queue, server->recv_buff);
	}
}

static void server_end(mpi_rpc_server_t *server) {
	server->server_thread_cancel = 1;
	destroy_mpi_rpc_server(server);
}

static void send_result(void *param, int source, int tag, int len) {
	rpc_head_t *head = zmalloc(sizeof(rpc_head_t));
	head->len = len;
	head->source = source;
	head->tag = tag;
	mpi_send(head, head->source, head->tag, sizeof(rpc_head_t));
	mpi_send(param, head->source, head->tag, head->len);
	zfree(head);
}

/*--------------------API Implementation--------------------*/
mpi_rpc_server_t *create_mpi_rpc_server(int thread_num, int rank, void *(*resolve_handler)(event_handler_t *event_handler, void* msg_queue))
{
	mpi_rpc_server_t *this = zmalloc(sizeof(mpi_rpc_server_t));

	this->request_queue = alloc_syn_queue(128, sizeof(common_msg_t));
	this->rank = rank;
	this->server_thread_cancel = 0;
	this->thread_pool = alloc_thread_pool(thread_num, this->request_queue, resolve_handler);

	this->op = zmalloc(sizeof(mpi_rpc_server_op_t));
	this->op->server_start = server_start;
	this->op->server_end = server_end;
	this->op->send_result = send_result;

	this->recv_buff = zmalloc(sizeof(common_msg_t));
	return this;
}

void destroy_mpi_rpc_server(mpi_rpc_server_t *server)
{
	server->server_thread_cancel = 1;
	destroy_syn_queue(server->request_queue);
	destroy_thread_pool(server->thread_pool);
	zfree(server->op);
	zfree(server->recv_buff);
	zfree(server);
}
