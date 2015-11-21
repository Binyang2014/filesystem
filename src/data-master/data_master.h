/*
 * data_master.h
 *
 *  Created on: 2015年7月7日
 *      Author: ron
 */

#ifndef SRC_DATA_MASTER_DATA_MASTER_H_
#define SRC_DATA_MASTER_DATA_MASTER_H_
#include <stddef.h>
#include <pthread.h>
#include "name_space.h"
#include "message.h"
#include "basic_list.h"
#include "rpc_server.h"
#include "machine_role.h"
#include "syn_tool.h"

typedef struct {
	int rank;
	sds visual_ip;
	unsigned long used_blocks;
	unsigned long free_blocks;
}storage_machine_sta_t;

struct data_master{
	int rank;
	int master_rank;
	sds visual_ip;
	size_t group_size;		//size
	name_space_t *namespace;
	syn_queue_t *storage_q;
	uint64_t free_size;
	uint64_t free_blocks;
	uint64_t global_id;
	rpc_server_t *rpc_server;
	pthread_mutex_t *mutex_data_master; //
};

struct data_master_op{
	int (*machine_register)(storage_machine_sta_t *machine_sta);
	void (*create_temp_file)(char *name);
	void (*append_temp_file)(event_handler_t *event_handler);
	void (*read_temp_file)(event_handler_t *event_handler);
	void (*delete_temp_file)(char *name); //unimplemented interface
};

typedef struct data_master data_master_t;

data_master_t* create_data_master(map_role_value_t *role, uint64_t free_flocks);
void destroy_data_master(data_master_t *this);
void *data_master_init(void *args);

#endif /* SRC_DATA_MASTER_DATA_MASTER_H_ */
