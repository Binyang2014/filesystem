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
static void execute(struct mpi_rpc_client *client, void *message, int message_size, int tag, void **buf, int *length, MPI_Status status) {
	MPI_Send(message, message_size, MPI_CHAR, client->target, tag, client->comm);
	MPI_Recv(length, 1, MPI_INT, client->target, tag, client->comm, &status);
	*buf = zmalloc(length);
	MPI_Recv(*buf, length, MPI_CHAR, client->target, tag, client->comm, &status);
}

/*---------------API Implementation----------------*/
mpi_rpc_client_t *create_mpi_rpc_client(MPI_Comm comm, int rank, int target) {
	mpi_rpc_client_t *this = zmalloc(sizeof(mpi_rpc_client_t));
	this->op = zmalloc(sizeof(mpi_rpc_client_op_t));
	this->comm = comm;
	this->rank = rank;
	this->target = target;

	this->op->execute = execute;
}

void destroy_mpi_rpc_client(mpi_rpc_client_t *client) {

}

