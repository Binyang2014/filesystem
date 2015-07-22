/*
 * sub_master.h
 *
 *  Created on: 2015年7月7日
 *      Author: ron
 */

#ifndef SRC_SUB_MASTER_SUB_MASTER_H_
#define SRC_SUB_MASTER_SUB_MASTER_H_
#include <stdint.h>
#include <pthread.h>
#include "../common/name_space.h"
#include "../common/sds.h"
#include "../common/communication/mpi_rpc_server.h"
#include "../common/communication/mpi_rpc_client.h"

struct data_server_des {

};

struct sub_master_op {

};

struct sub_master{
	machine_role_t *role;
	name_space_t *name_space;
	mpi_rpc_server_t *rpc_server;
	mpi_rpc_client_t *rpc_client;
	uint64_t sub_global_id;
	pthread_mutex_t mutex_id;
};

typedef struct data_server_des data_server_des_t;
typedef struct sub_master sub_master_t;
typedef struct data_server_des data_server_des_t;


#endif /* SRC_SUB_MASTER_SUB_MASTER_H_ */
