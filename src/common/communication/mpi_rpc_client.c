/*
 * mpi_rpc_client.c
 *
 *  Created on: 2015年7月13日
 *      Author: ron
 */
#include "mpi_rpc_client.h"
#include "../structure_tool/zmalloc.h"
#include "mpi_rpc_structure.h"
/*--------------------Private Declaration---------------------*/
static void* execute(mpi_rpc_client_t *client, void *message, int message_size, int tag);

/*--------------------Global Declaration----------------------*/
mpi_rpc_client_t *create_mpi_rpc_client(int rank, int target);

/*--------------------Private Implementation------------------*/
static void* execute(mpi_rpc_client_t *client, void *message, int message_size, int tag) {
	mpi_send(message, client->target, tag, message_size);
	rpc_head_t *head = zmalloc(sizeof(rpc_head_t));
	mpi_recv(head, client->target, tag, sizeof(rpc_head_t), &(client->status));
	void *buf = zmalloc(head->len);
	mpi_recv(buf, client->target, head->tag, head->len, &(client->status));
	zfree(head);
	return buf;
}

/*---------------------API Implementation----------------------*/
mpi_rpc_client_t *create_mpi_rpc_client(int rank, int target) {
	mpi_rpc_client_t *this = zmalloc(sizeof(mpi_rpc_client_t));
	this->op = zmalloc(sizeof(mpi_rpc_client_op_t));
	this->rank = rank;
	this->target = target;

	this->op->execute = execute;
	return this;
}

void destroy_mpi_rpc_client(mpi_rpc_client_t *client) {
	zfree(client->op);
	zfree(client);
}

