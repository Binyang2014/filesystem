/*
 * mpi_rpc_client.c
 *
 *  Created on: 2015年7月13日
 *      Author: ron
 */
#include "mpi_rpc_client.h"
#include "../zmalloc.h"
#include "mpi_rpc_structure.h"

/*---------------Private Declaration---------------*/
static void* execute(mpi_rpc_client_t *client, void *message, int message_size, int tag) {
	MPI_Status status;

	MPI_Send(message, message_size, MPI_CHAR, client->target, tag, client->comm);
	puts("send request");
	rpc_head_t *head = zmalloc(sizeof(rpc_head_t));
	MPI_Recv(head, sizeof(rpc_head_t), MPI_CHAR, client->target, tag, client->comm, &status);
	puts("receive head");
	void *buf = zmalloc(head->len);
	MPI_Recv(buf, head->len, MPI_CHAR, client->target, tag, client->comm, &status);
	puts("buf");
	zfree(head);
	return buf;
}

/*---------------API Implementation----------------*/
mpi_rpc_client_t *create_mpi_rpc_client(MPI_Comm comm, int rank, int target) {
	mpi_rpc_client_t *this = zmalloc(sizeof(mpi_rpc_client_t));
	this->op = zmalloc(sizeof(mpi_rpc_client_op_t));
	this->comm = comm;
	this->rank = rank;
	this->target = target;

	this->op->execute = execute;
	return this;
}

void destroy_mpi_rpc_client(mpi_rpc_client_t *client) {

}

