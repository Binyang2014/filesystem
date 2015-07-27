/*
 * mpi_rpc_client.h
 *
 *  Created on: 2015年7月13日
 *      Author: ron
 */

#ifndef SRC_COMMON_COMMUNICATION_MPI_RPC_CLIENT_H_
#define SRC_COMMON_COMMUNICATION_MPI_RPC_CLIENT_H_
#include <stddef.h>
#include <stdint.h>
#include <mpi.h>

struct mpi_rpc_client {
	mpi_status_t status;
	int client_id;
	int target;
	struct mpi_rpc_client_op *op;
};

struct mpi_rpc_client_op {
	void *(*execute)(struct mpi_rpc_client *client, void *message, int message_size, int tag);
};

typedef struct mpi_rpc_client mpi_rpc_client_t;
typedef struct mpi_rpc_client_op mpi_rpc_client_op_t;

mpi_rpc_client_t *create_mpi_rpc_client(int client_id, int target);
void destroy_mpi_rpc_client(mpi_rpc_client_t *client);
#endif /* SRC_COMMON_COMMUNICATION_MPI_RPC_CLIENT_H_ */
