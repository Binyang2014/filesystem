/*
 * data_master_request.h
 *
 *  Created on: 2015年11月12日
 *      Author: ron
 */

#ifndef SRC_DATA_MASTER_DATA_MASTER_REQUEST_H_
#define SRC_DATA_MASTER_DATA_MASTER_REQUEST_H_

#include <mpi.h>
#include "rpc_client.h"
#include "message.h"

struct data_master_request{
	rpc_client_t *client;
	struct data_master_request_op *op;
};

struct data_master_request_op{
	void (*create_tmp_file)(struct data_master_request *request, client_create_file_t *c_cmd);
	void *(*append_temp_file)(struct data_master_request *request, client_append_file_t *c_cmd);
	void *(*read_temp_file)(struct data_master_request *request, client_read_file_t *c_cmd);
	void (*register_to_master)(struct data_master_request *request, c_d_register_t *c_cmd);
	void (*stop_master)(struct data_master_request *request);
};

typedef struct data_master_request data_master_request_t;
typedef struct data_master_request_op data_master_request_op_t;

data_master_request_t *create_data_master_request(int client_id, int target, int tag);
void destroy_data_master_request(data_master_request_t *request);

#endif /* SRC_DATA_MASTER_DATA_MASTER_REQUEST_H_ */
