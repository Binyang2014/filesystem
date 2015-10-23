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
#include "../common/structure_tool/syn_tool.h"
#include "../common/structure_tool/threadpool.h"
#include "../common/structure_tool/zmalloc.h"
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

/**
 * char * to integer
 */
static inline int local_atoi(char *c) {
	int s = 0;
	char *p = c;
	while(*p){
		s = (s << 3) + (s << 1) + *p++ - '0';
	}
	return s;
}

/**
 * get key value
 * @type if type = 0 comment line
 * 		 else if type = 1 blank line
 * 		 else param line
 * @key save key
 * @value save value
 */
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

/**
 * get key and value
 * skip blank and comment line
 */
static void inline get_line_param(FILE *fp, char *buf, char *key, char *value) {
	int type = 0;
	do {
		char *c = fgets(buf, LINE_LENGTH, fp);
		assert(c != NULL);
		get_param(buf, key, value, &type);
	}while(0 == type || 1 == type);
}

/**
 * get param and translate it into integer
 */
static void inline read_conf_int(FILE *fp, char *buf, char *key, char *value, int *v) {
	get_line_param(fp, buf, key, value);
	*v = local_atoi(value);
}

/**
 * get param and translate it into string
 */
static void inline read_conf_str(FILE *fp, char *buf, char *key, char *value, char *v) {
	get_line_param(fp, buf, key, value);
	strcpy(v, value);
}

static inline map_role_value_t *create_role(char *master_ip, char *ip, machine_role_e type){
	map_role_value_t *role = zmalloc(sizeof(*role));
	role->type = type;
	strcpy(role->master_ip, master_ip);
	strcpy(role->ip, ip);
	return role;
}

static inline map_role_value_t *create_data_master_role(char *master_ip, char *ip) {
	return create_role(master_ip, ip, DATA_MASTER);
}

static inline map_role_value_t *create_data_server_role(char *master_ip, char *ip) {
	return create_role(master_ip, ip, DATA_SERVER);
}

static void get_net_topology(machine_role_allocator_t *allocator) {
	FILE *fp = fopen(allocator->file_path, "r");

	//verify file exists
	assert(fp != NULL);

	char buf[LINE_LENGTH] = {};
	char key[LINE_LENGTH] = {};
	char value[LINE_LENGTH] = {};
	sds map_key;
	char ip[LINE_LENGTH] = {};
	int current_group_type = 0;
	int current_group_size = 0;
	char current_master[LINE_LENGTH] = {};
	int group_num = 0;

	//read group number
	read_conf_int(fp, buf, key, value, &group_num);

	while(group_num--) {
		read_conf_int(fp, buf, key, value, &current_group_size);
		read_conf_str(fp, buf, key, value, current_master);
		map_key = sds_new(value);
		allocator->roles->op->put(allocator->roles, map_key, create_data_master_role(current_master, value));
		sds_free(map_key);

		while(current_group_size--) {
			get_line_param(fp, buf, key, value);
			map_key = sds_new(value);
			allocator->roles->op->put(allocator->roles, map_key, create_data_server_role(current_master, value));
			sds_free(map_key);
		}
	}

	fclose(fp);
}

/**
 * distribute the machine role
 */
static void allocate_machine_role(machine_role_allocator_t *allocator) {
	map_t *roles = local_allocator->roles;

	map_iterator_t *iter = create_map_iterator(roles);
	map_role_value_t *role;
	map_role_value_t *master_role;
	while(iter->op->has_next(iter)){
		role = iter->op->next(iter);
		sds master_ip = sds_new(role->master_ip);
		master_role = (map_role_value_t *)(roles->op->get(roles, master_ip));
		role->master_rank = master_role->rank;
		allocator->server->op->send_result(role, allocator->rank, 1, sizeof(*role), ANS);
		sds_free(master_ip);
	}

	destroy_map_iterator(iter);
}

static void *get_event_handler_param(event_handler_t *event_handler) {
	return event_handler->event_buffer_list->head->value;
}

//TODO lock this
/**
 * get machine role
 */
static void machine_register(event_handler_t *event_handler) {
	machine_register_role_t *param = get_event_handler_param(event_handler);
	map_role_value_t *role = zmalloc(sizeof(*role));

	sds key_ip = sds_new(param->ip);
	strcpy(role->ip, param->ip); //copy role ip
	role->rank = param->source; //copy role rank
	role->type = param->role_type;
	local_allocator->roles->op->put(local_allocator->roles, key_ip, role);
	local_allocator->register_machine_num++;
	if(local_allocator->register_machine_num == local_allocator->machine_num){
		allocate_machine_role(local_allocator);
	}
	sds_free(key_ip);
}

static void *resolve_handler(event_handler_t *event_handler, void* msg_queue) {
	common_msg_t common_msg;
	syn_queue_t* queue = msg_queue;
	queue->op->syn_queue_pop(queue, &common_msg);
	switch(common_msg.operation_code)
	{
		case MACHINE_ROLE_GET_ROLE:
			event_handler->handler = machine_register;
			break;
		default:
			event_handler->handler = NULL;
	}
	return event_handler->handler;
}

/**
 *
 */
machine_role_allocator_t *create_machine_role_allocater(size_t size, int rank, char *file_path) {
	machine_role_allocator_t *this = zmalloc(sizeof(machine_role_allocator_t));
	strcpy(this->file_path, file_path);
	local_allocator = this;
	this->machine_num = size;
	this->register_machine_num = 0;
	this->roles = create_map(size * 2, NULL, NULL, NULL, map_list_pair_free);
	this->op = zmalloc(sizeof(machine_role_allocator_op_t));

	this->op->get_net_topology = get_net_topology;
	this->op->machine_register = machine_register;
	this->server = create_rpc_server(10, 1024, rank, resolve_handler);
	return this;
}

void destroy_machine_role_allocater(machine_role_allocator_t *this) {
	destroy_map(this->roles);
	destroy_rpc_server(this->server);
	zfree(this->op);
	zfree(this);
}

/*
 *
  mpicc -o role machine_role.c ../common/structure_tool/zmalloc.c
  ../common/structure_tool/threadpool.c ../common/structure_tool/sds.c
  ../common/structure_tool/basic_list.c ../common/structure_tool/map.c
  ../common/structure_tool/basic_queue.c ../common/structure_tool/log.c
  ../common/communication/rpc_server.c
 */
#define MACHINE_ROLE_TEST 1
#if defined(MACHINE_ROLE_TEST) || defined(GLOBAL_TEST)
int main() {
	machine_role_allocator_t *allocator = create_machine_role_allocater(10, 0, "topo.conf");
	allocator->op->get_net_topology(allocator);
	destroy_machine_role_allocater(allocator);
}
#endif
