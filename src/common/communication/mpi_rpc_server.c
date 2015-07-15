/*
 * mpi_rpc.c
 *
 *  Created on: 2015年7月10日
 *      Author: ron
 */

#include <math.h>
#include "mpi_rpc_server.h"
#include "../zmalloc.h"

/*--------------------Private Implementation--------------------*/
static void server_start(mpi_rpc_server_t *server) {
	server->thread_pool->tp_ops->start(server->thread_pool);
	puts("hello ");
//	while(!server->server_thread_cancel) {
//		//TODO if multi client send request to this, Is there any parallel problem?
//		MPI_Recv(server->recv_buff, MAX_CMD_MSG_LEN, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &(server->status));
//		puts("receive recv_buff");
//		server->request_queue->op->syn_queue_push(server->request_queue, server->recv_buff);
//		puts("put message into queue");
//	}
}

static void server_end(mpi_rpc_server_t *server) {
	server->server_thread_cancel = 1;
}

void send_result(void **param) {
	rpc_head_t *head = *param;
	MPI_Send(head, sizeof(*head), MPI_CHAR, head->source, head->tag, MPI_COMM_WORLD);
	MPI_Send(*(param + 1), head->len, MPI_CHAR, head->source, head->tag, MPI_COMM_WORLD);
}

/*--------------------API Implementation--------------------*/
mpi_rpc_server_t *create_mpi_rpc_server(int thread_num, int rank, void *(*resolve_handler)(event_handler_t *event_handler, void* msg_queue)) {
	mpi_rpc_server_t *this = zmalloc(sizeof(mpi_rpc_server_t));

	this->request_queue = alloc_syn_queue(128, MAX_CMD_MSG_LEN);
	this->comm = MPI_COMM_WORLD;
	this->rank = rank;
	this->server_thread_cancel = 0;
	this->thread_pool = alloc_thread_pool(thread_num, this->request_queue->queue, resolve_handler);

	this->op = zmalloc(sizeof(mpi_rpc_server_op_t));
	this->op->server_start = server_start;
	this->op->server_end = server_end;
	this->op->send_result = send_result;

	this->recv_buff = zmalloc(MAX_CMD_MSG_LEN);
	return this;
}

void destroy_mpi_rpc_server(mpi_rpc_server_t *server) {
	server->op->server_end(server);
}

#define MPI_RPC_TEST 1

#if defined(GLOBAL_TEST) || defined(MPI_RPC_TEST)
mpi_rpc_server_t *local_server;
#include <stdio.h>
#include <stdlib.h>
#include "mpi_rpc_client.h"
#include "message.h"
syn_queue_t* queue;

typedef struct {
	uint16_t code;
	int source;
	int tag;
	char rest[4094];
}test_rpc_t;

typedef struct {
	int result;
}test_result_t;

void hello(event_handler_t *event_handler) {
	puts("hello start");

	void **result = zmalloc(sizeof(void *) * 2);

	*result = zmalloc(sizeof(rpc_head_t));
	((rpc_head_t *)(*result))->len = sizeof(test_result_t);
	((rpc_head_t *)(*result))->source = 1;
	((rpc_head_t *)(*result))->tag = 1;

	*(result + 1) = zmalloc(sizeof(test_result_t));
	((test_result_t *)(*(result + 1)))->result = 1090;
	local_server->op->send_result(result);

	puts("hello end");
}

void *resolve_handler(event_handler_t *event_handler, void *msg_queue) {
	common_msg_t common_msg;
	queue->op->syn_queue_pop(queue, &common_msg);
	switch(common_msg.operation_code)
	{
		case 1:
			puts("case 1");
			event_handler->handler = hello;
			break;
		default:
			event_handler->handler = NULL;
	}
	return event_handler->handler;
}

int main(argc, argv)
	int argc;
	char ** argv;
{
	int rank, size, provided;

	MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if(rank == 0) {
		mpi_rpc_server_t *server = create_mpi_rpc_server(3, 0, resolve_handler);
		puts("server 123");
		local_server = server;
		server->op->server_start(server);

	destroy_syn_queue(queue_syn);
	}else {
		mpi_rpc_client_t *client = create_mpi_rpc_client(MPI_COMM_WORLD, rank, 0);
		test_rpc_t *msg = zmalloc(sizeof(test_rpc_t));
		msg->code = 1;
		msg->source = 1;
		msg->tag = 1;
//		puts("client 123");
		test_result_t *result = client->op->execute(client, msg, 4096, 1);
		printf("rpc test result = %d\n", result->result);
	}
	puts("MPI_Finalize");
	MPI_Finalize();
	return 0;
}

#endif
