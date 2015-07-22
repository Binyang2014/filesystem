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
#include "./common/communication/message.h"

struct sub_master_status {
	char ip[16];
	int status;
	int rank;
};

struct master_sub_masters {

};

struct master_op {
	void (*master_start)(struct master *master);
};

struct master {
	MPI_Comm comm;
	int rank;
	int machine_num;
	int register_machin_num;
	name_space_t *name_space;
	pthread_t *thread;
	mpi_rpc_server_t *rpc_server;
	mpi_rpc_client_t *rpc_client;
	uint64_t global_id;
	pthread_mutex_t *mutex_global_id;
};

typedef struct master master_t;
typedef struct sub_master_status sub_master_status_t;

/**
 * size machine size
 */
master_t *create_master(size_t size, int rank, int machine_num);
void master_get_net_topology(master_t *master, sds file_name);
void destroy_master(master_t *master);

#endif /* SRC_MASTER_MASTER_H_ */
