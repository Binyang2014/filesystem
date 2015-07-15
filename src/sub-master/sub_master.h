/*
 * sub_master.h
 *
 *  Created on: 2015年7月7日
 *      Author: ron
 */

#ifndef SRC_SUB_MASTER_SUB_MASTER_H_
#define SRC_SUB_MASTER_SUB_MASTER_H_

#include "../common/name_space.h"
#include "../common/sds.h"
#include "../common/communication/mpi_rpc_server.h"
#include "../common/communication/mpi_rpc_client.h"

typedef struct {
	name_space_t *name_space;
	mpi_rpc_server_t *rpc_server;
	mpi_rpc_client_t *rpc_client;
};

#endif /* SRC_SUB_MASTER_SUB_MASTER_H_ */
