/*
 * master.h
 *
 *  Created on: 2015年7月7日
 *      Author: ron
 */

#ifndef SRC_MASTER_MASTER_H_
#define SRC_MASTER_MASTER_H_
#include <mpi.h>
#include <pthread.h>
#include "./common/name_space.h"
#include "./common/communication/mpi_rpc_server.h"
#include "./common/communication/mpi_rpc_client.h"

struct master_op {
	void (*master_start)(struct master *master);
};

struct master {
	MPI_Comm comm;
	int rank;
	name_space_t *name_space;
	pthread_t *thread;
	mpi_rpc_server_t *rpc_server;
	mpi_rpc_client_t *rpc_client;
};

typedef struct master master_t;

/**
 * size machine size
 */
master_t *create_master(size_t size);
void destroy_master(master_t *master);

#endif /* SRC_MASTER_MASTER_H_ */
