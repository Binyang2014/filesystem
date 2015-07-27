/*
 * mpi_rpc_server.h
 *
 *  Created on: 2015年7月10日
 *      Author: ron
 */

#ifndef SRC_COMMON_COMMUNICATION_MPI_RPC_SERVER_H_
#define SRC_COMMON_COMMUNICATION_MPI_RPC_SERVER_H_

#include <stdint.h>
#include <mpi.h>
#include "message.h"
#include "../structure_tool/threadpool.h"
#include "../../global.h"

struct mpi_rpc_server {
	int server_id;
	int server_thread_cancel;
	void *recv_buff;
	struct mpi_rpc_server_op *op;
	syn_queue_t *request_queue;
	thread_pool_t *thread_pool;
};

struct mpi_rpc_server_op {
	void (*server_start)(struct mpi_rpc_server *server);
	void (*server_stop)(struct mpi_rpc_server *server);
	int (*send_result)(void *param, int source, int tag, int len,
			reply_msg_type_t type);
};

typedef struct mpi_rpc_server mpi_rpc_server_t;
typedef struct mpi_rpc_server_op mpi_rpc_server_op_t;

mpi_rpc_server_t *create_mpi_rpc_server(int thread_num, int server_id, void *(*resolve_handler)(event_handler_t *event_handler, void* msg_queue));
void destroy_mpi_rpc_server(mpi_rpc_server_t *server);

#endif /* SRC_COMMON_COMMUNICATION_MPI_RPC_SERVER_H_ */
