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
#include "../common/communication/rpc_client.h"
#include "../common/communication/message.h"
#include "../common/structure_tool/map.h"

struct map_role_value {
	char master_ip[16];
	char sub_master_ip[16];
	char data_master_ip[16];
	enum machine_role type;
	int rank;
};

struct machine_role_allocator {
	char file_path[255];
	char master_ip[16];
	int master_rank;
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

machine_role_allocator_t *create_machine_role_allocater(size_t size, char *file_path);
void destroy_machine_role_allocater(machine_role_allocator_t *this);

#endif /* SRC_MASTER_MACHINE_ROLE_H_ */
