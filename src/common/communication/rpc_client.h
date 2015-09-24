/*
 * rpc_client.h
 *
 *  Created on: 2015年7月13日
 *      Author: ron
 */

#ifndef SRC_COMMON_COMMUNICATION_RPC_CLIENT_H_
#define SRC_COMMON_COMMUNICATION_RPC_CLIENT_H_
#include <stddef.h>
#include <stdint.h>

enum execute_type
{
	READ_C_TO_D,
	WRITE_C_TO_D,
	COMMAND,
	COMMAND_WITH_RETURN,
	COMMAND_WITHOUT_RETURN,
	STOP_SERVER
};

struct rpc_client {
	int client_id;
	int target;
	int tag;
	uint32_t second_send_buff_len;
	void* send_buff;
	void* second_send_buff;
	void* recv_buff;
	uint32_t recv_buff_len;
	struct rpc_client_op *op;
};

struct rpc_client_op {
	int (*execute)(struct rpc_client *client, enum execute_type exe_type);
	void (*set_recv_buff)(struct rpc_client* , void* , uint32_t);
	void (*set_send_buff)(struct rpc_client* , void* );
	void (*set_second_send_buff)(struct rpc_client*, void*, uint32_t);
};

typedef struct rpc_client rpc_client_t;
typedef struct rpc_client_op rpc_client_op_t;
typedef enum execute_type execute_type_t;

rpc_client_t *create_rpc_client(int client_id, int target, int tag);
void destroy_rpc_client(rpc_client_t *client);
#endif /* SRC_COMMON_COMMUNICATION_MPI_RPC_CLIENT_H_ */
