/*
 * machine_role.h
 *
 *  Created on: 2015年7月18日
 *      Author: ron
 */

#ifndef SRC_MASTER_MACHINE_ROLE_H_
#define SRC_MASTER_MACHINE_ROLE_H_
#include <stdint.h>
#include "../common/communication/rpc_server.h"
#include "../common/communication/message.h"
#include "../common/structure_tool/map.h"
#include "../common/structure_tool/sds.h"

struct map_role_value {
	char master_ip[16];
	char ip[16];
	machine_role_e type;
	int master_rank;
	int rank;
	size_t group_size;
};

struct machine_role_allocator {
	int rank;
	char file_path[255];
	size_t register_machine_num;
	size_t machine_num; //machine number
	rpc_server_t *server;
	map_t *roles;
	struct machine_role_allocator_op *op;
};

struct machine_role_allocator_op {
	void (*get_net_topology)(struct machine_role_allocator *allocator);
	void (*machine_register)(event_handler_t *event_handler);
};

typedef struct machine_role_allocator machine_role_allocator_t;
typedef struct machine_role_allocator_op machine_role_allocator_op_t;
typedef struct map_role_value map_role_value_t;

machine_role_allocator_t *create_machine_role_allocater(size_t size, int rank, char *file_path);
void destroy_machine_role_allocater(machine_role_allocator_t *this);


/*************************** Machine Role Fetcher ****************/
struct machine_role_fetcher{
	int rank;
	rpc_client_t *client;
	struct machine_role_fetcher_op *op;
};

struct machine_role_fetcher_op{
	void (*register_to_zero_rank)(struct machine_role_fetcher *fetcher); // register this mpi process info to zero process
};

typedef struct machine_role_fetcher machine_role_fetcher_t;
typedef struct machine_role_fetcher_op machine_role_fetcher_op_t;


machine_role_fetcher_t *create_machine_role_fetcher(int rank);
void destroy_machine_role_fetcher(machine_role_fetcher_t *fetcher);

#endif /* SRC_MASTER_MACHINE_ROLE_H_ */

