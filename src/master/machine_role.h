/*
 * machine_role.h
 *
 *  Created on: 2015年7月18日
 *      Author: ron
 */

#ifndef SRC_MASTER_MACHINE_ROLE_H_
#define SRC_MASTER_MACHINE_ROLE_H_
#include <stdint.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "rpc_server.h"
#include "rpc_client.h"
#include "message.h"
#include "map.h"
#include "sds.h"

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
	char *net_name;
	size_t register_machine_num;
	size_t machine_num; //machine number
	rpc_server_t *server;
	map_t *roles;
	struct machine_role_allocator_op *op;
	pthread_mutex_t *mutex_allocator;
	struct map_role_value *role;
};

struct machine_role_allocator_op {
	void (*get_net_topology)(struct machine_role_allocator *allocator);
	void (*machine_register)(event_handler_t *event_handler);
};

typedef struct machine_role_allocator machine_role_allocator_t;
typedef struct machine_role_allocator_op machine_role_allocator_op_t;
typedef struct map_role_value map_role_value_t;


map_role_value_t *machine_role_allocator_start(size_t size, int rank, char *file_path, char *net_name);
void get_visual_ip(const char *net_name, char *ip);

/*************************** Machine Role Fetcher ****************/
struct machine_role_fetcher{
	int rank;
	rpc_client_t *client;
	char net_name[INET6_ADDRSTRLEN];
	struct machine_role_fetcher_op *op;
};

struct machine_role_fetcher_op{
	map_role_value_t *(*register_to_zero_rank)(struct machine_role_fetcher *fetcher); // register this mpi process info to zero process
};

typedef struct machine_role_fetcher machine_role_fetcher_t;
typedef struct machine_role_fetcher_op machine_role_fetcher_op_t;

map_role_value_t *get_role(int rank, char *net_name);

#endif /* SRC_MASTER_MACHINE_ROLE_H_ */

