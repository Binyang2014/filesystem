/*
 * machine_role.c
 *
 *  Created on: 2015年7月18日
 *      Author: ron
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "machine_role.h"
#include "../common/syn_tool.h"
#include "../common/threadpool.h"
#include "../common/zmalloc.h"
#include "../common/communication/message.h"

#define LINE_LENGTH 64
/*---------------------Private Declaration---------------------*/
static machine_role_allocator_t *local_allocator;
static const char blank_space = ' ';

static void map_list_pair_free(void *value) {
	pair_t *p = value;
	sds_free(p->key);
//	map_role_value_t *v = p->value;
//	puts("\n----print role----");
//	printf("ip = %s\nmaster_ip = %s\nsub_master_ip = %s\ndata_master_ip = %s\ntype = %d\n", p->key, v->master_ip, v->sub_master_ip, v->data_master_ip, v->type);
//	puts("----print role end----");
	zfree(p->value);
	zfree(p);
}

static inline int local_atoi(char *c) {
	int s = 0;
	char *p = c;
	while(*p){
		s = (s << 3) + (s << 1) + *p++ - '0';
	}
	return s;
}

static inline void get_param(char *p, char *key, char *value, int *line_type) {
	int index = 0;

	char *line = p;
	while(*line == blank_space || *line == '\t') line++;

	if('#' == *line)  {
		*line_type = 0; //commnet
	}else if(*line == '\n'){
		*line_type = 1; //blank line
	}else{
		while(blank_space != *line && '=' != *line && '\t' != *line ) {
			*(key + index++) = *line++;
		}

		*(key + index) = 0;
		index = 0;

		while(blank_space == *line || '=' == *line || '\t' == *line )line++;

		while(blank_space != *line && '\n' != *line && '\t' != *line ) {
			*(value + index++) = *line++;
		}
		*(value + index++) = 0;

		*line_type = 2;
	}
}

static void inline get_line_param(FILE *fp, char *buf, char *key, char *value) {
	int type = 0;
	do {
		char *c = fgets(buf, LINE_LENGTH, fp);
		assert(c != NULL);
		get_param(buf, key, value, &type);
	}while(0 == type || 1 == type);
}

static void inline read_conf_int(FILE *fp, char *buf, char *key, char *value, int *v) {
	get_line_param(fp, buf, key, value);
	*v = local_atoi(value);
}

static void inline read_conf_str(FILE *fp, char *buf, char *key, char *value, char *v) {
	get_line_param(fp, buf, key, value);
	strcpy(v, value);
}

static inline map_role_value_t *create_sub_master_role() {
	map_role_value_t *role = zmalloc(sizeof(*role));
	//strcpy(role->master_ip, master_ip);
	role->type = SUB_MASTER;
	return role;
}

static inline map_role_value_t *create_data_master_role(char *sub_master_ip) {
	map_role_value_t *role = zmalloc(sizeof(*role));
	strcpy(role->sub_master_ip, sub_master_ip);
	role->type = DATA_MASTER;
	return role;
}

static inline map_role_value_t *create_data_server_role(char *sub_master_ip, char *data_master_ip) {
	map_role_value_t *role = zmalloc(sizeof(*role));
	strcpy(role->sub_master_ip, sub_master_ip);
	strcpy(role->data_master_ip, data_master_ip);
	role->type = DATA_SERVER;
	return role;
}

static void get_net_topology(machine_role_allocator_t *allocator) {
	FILE *fp = fopen(allocator->file_path, "r");

	assert(fp != NULL);

	char buf[LINE_LENGTH] = {};
	char key[LINE_LENGTH] = {};
	char value[LINE_LENGTH] = {};
	sds map_key;
	char ip[LINE_LENGTH] = {};
	int current_group_type = 0;
	int current_group_size = 0;
	char current_sub_master[LINE_LENGTH] = {};
	char current_data_master[LINE_LENGTH] = {};
	int group_num = 0;

	read_conf_str(fp, buf, key, value, allocator->master_ip);
	read_conf_int(fp, buf, key, value, &group_num);

	read_conf_int(fp, buf, key, value, &current_group_type);
	read_conf_int(fp, buf, key, value, &current_group_size);

	while(current_group_size--) {
		get_line_param(fp, buf, key, value);
		map_key = sds_new(value);
		allocator->roles->op->put(allocator->roles, map_key, create_sub_master_role());
		sds_free(map_key);
	}

	group_num--;
	while(group_num--) {
		read_conf_int(fp, buf, key, value, &current_group_type);
		read_conf_int(fp, buf, key, value, &current_group_size);
		read_conf_str(fp, buf, key, value, current_sub_master);
		read_conf_str(fp, buf, key, value, current_sub_master);
		map_key = sds_new(current_sub_master);
		allocator->roles->op->put(allocator->roles, map_key, create_data_master_role(current_sub_master));
		sds_free(map_key);

		while(current_group_size--) {
			get_line_param(fp, buf, key, value);
			map_key = sds_new(value);
			allocator->roles->op->put(allocator->roles, map_key, create_data_server_role(current_sub_master, current_data_master));
			sds_free(map_key);
		}
	}

	fclose(fp);
}

static void allocate_machine_role(machine_role_allocator_t *allocator) {

}

static void *get_event_handler_param(event_handler_t *event_handler) {
	return event_handler->event_buffer_list->head->value;
}

//TODO lock this
static void get_machine_role(event_handler_t *event_handler) {
	machine_register_role_t *param = get_event_handler_param(event_handler);
	sds key_ip = sds_new(param->ip);
	if(strcpy(param->ip, local_allocator) == 0) {
		local_allocator->master_rank = param->source;
		return;
	}
	map_role_value_t *result = local_allocator->roles->op->get(local_allocator->roles, key_ip);
	result->rank = param->source;
	local_allocator->server->op->send_result(result, param->source, param->tag, sizeof(machine_role_t));
	sds_free(key_ip);
	local_allocator->register_machine_num++;
}

void *resolve_handler(event_handler_t *event_handler, void* msg_queue) {
	common_msg_t common_msg;
	syn_queue_t* queue = msg_queue;
	queue->op->syn_queue_pop(queue, &common_msg);
	switch(common_msg.operation_code)
	{
		case MACHINE_ROLE_GET_ROLE:
			event_handler->handler = get_machine_role;
			break;
		default:
			event_handler->handler = NULL;
	}
	return event_handler->handler;
}

machine_role_allocator_t *create_machine_role_allocater(size_t size, int rank, char *file_path) {
	machine_role_allocator_t *this = zmalloc(sizeof(machine_role_allocator_t));
	strcpy(this->file_path, file_path);
	local_allocator = this;
	this->machine_num = size;
	this->register_machine_num = 0;
	this->roles = create_map(size * 2, NULL, NULL, NULL, map_list_pair_free);
	this->op = zmalloc(sizeof(machine_role_allocator_op_t));

	this->op->get_net_topology = get_net_topology;
	this->op->get_machine_role = get_machine_role;
	this->server = create_mpi_rpc_server(10, rank, resolve_handler);
	return this;
}

void destroy_machine_role_allocater(machine_role_allocator_t *this) {
	destroy_map(this->roles);
	destroy_mpi_rpc_server(this->server);
	zfree(this->op);
	zfree(this);
}

#define MACHINE_ROLE_TEST 1
#if defined(MACHINE_ROLE_TEST) || defined(GLOBAL_TEST)
int main() {
	machine_role_allocator_t *allocator = create_machine_role_allocater(10, 0, "topo.conf");
	allocator->op->get_net_topology(allocator);
	destroy_machine_role_allocater(allocator);
}
#endif
