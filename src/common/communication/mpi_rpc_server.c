/*
 * mpi_rpc.c
 *
 *  Created on: 2015年7月10日
 *      Author: ron
 */

#include "mpi_rpc_server.h"
#include "../zmalloc.h"

/*--------------------Private Implementation--------------------*/
static void server_start(mpi_rpc_server_t *server) {
	while(!server->server_thread_cancel) {
		//TODO if multi client send request to this, Is there any parallel problem?
		MPI_Recv(server->recv_buff, sizeof(pre_order_t), MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, server->status);
		server->request_queue->op->syn_queue_push(server->request_queue, server->recv_buff);
	}
}

static void server_end(mpi_rpc_server_t *server) {
	server->server_thread_cancel = 1;
}

/*--------------------API Implementation--------------------*/
mpi_rpc_server_t *create_mpi_rpc_server(int thread_num, int rank, void (*resolve_handler)(thread_pool_t* thread_pool, void* msg_queue)) {
	mpi_rpc_server_t *this = zmalloc(sizeof(mpi_rpc_server_t));

	this->request_queue = alloc_syn_queue(128, sizeof(void *));
	this->comm = MPI_COMM_WORLD;
	this->rank = rank;
	this->server_thread_cancel = 0;
	this->thread_pool = alloc_thread_pool(thread_num, this->request_queue, resolve_handler);

	this->op = zmalloc(sizeof(mpi_rpc_server_op_t));
	this->op->server_start = server_start;
	this->op->server_end = server_end;
}

void destroy_mpi_rpc_server(mpi_rpc_server_t *server) {
	server->op->server_end(server);
}

#define MPI_RPC_TEST 1

#if defined(GLOBAL_TEST) || defined(MPI_RPC_TEST)
int main() {

}

#endif
