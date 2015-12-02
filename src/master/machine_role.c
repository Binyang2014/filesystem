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
#include <sys/types.h>
#include <ifaddrs.h>
#include "log.h"
#include "machine_role.h"
#include "syn_tool.h"
#include "threadpool.h"
#include "zmalloc.h"
#include "message.h"
#include "test_help.h"

#define LINE_LENGTH 64
/*---------------------Private Declaration---------------------*/
static machine_role_allocator_t *local_allocator;
static const char blank_space = ' ';

static void map_list_pair_free(void *value);
static inline int local_atoi(char *c);
static inline void get_param(char *p, char *key, char *value, int *line_type);
static void inline get_line_param(FILE *fp, char *buf, char *key, char *value);
static void inline read_conf_int(FILE *fp, char *buf, char *key, char *value, int *v);
static void inline read_conf_str(FILE *fp, char *buf, char *key, char *value, char *v);
static inline map_role_value_t *create_role(char *master_ip, char *ip, machine_role_e type);
static inline map_role_value_t *create_data_master_role(char *master_ip, char *ip, size_t group_size);
static inline map_role_value_t *create_data_server_role(char *master_ip, char *ip);
static void get_net_topology(machine_role_allocator_t *allocator);
static void allocate_machine_role(machine_role_allocator_t *allocator);
static void *get_event_handler_param(event_handler_t *event_handler);
static void machine_register(event_handler_t *event_handler);
static void *resolve_handler(event_handler_t *event_handler, void* msg_queue);
static machine_role_allocator_t *create_machine_role_allocator(size_t size, int rank, char *file_path, char *net_name);
static void destroy_machine_role_allocater(machine_role_allocator_t *this);
static map_role_value_t* register_to_zero_rank(struct machine_role_fetcher *fetcher);
static machine_role_fetcher_t *create_machine_role_fetcher(int rank, char *net_name);
static void destroy_machine_role_fetcher(machine_role_fetcher_t *fetcher);
extern map_role_value_t *get_role(int rank, char *net_name);
void get_visual_ip(const char *net_name, char *ip);

/*-----------------------------Implementation--------------------*/
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

static inline map_role_value_t *create_data_master_role(char *master_ip, char *ip, size_t group_size) {
	map_role_value_t *role =  create_role(master_ip, ip, DATA_MASTER);
	role->group_size = group_size;
	return role;
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
	int current_group_size = 0;
	char current_master[LINE_LENGTH] = {};
	int group_num = 0;

	//read group number
	read_conf_int(fp, buf, key, value, &group_num);

	while(group_num--) {
		read_conf_int(fp, buf, key, value, &current_group_size);
		read_conf_str(fp, buf, key, value, current_master);
		map_key = sds_new(value);
		allocator->roles->op->put(allocator->roles, map_key, create_data_master_role(current_master, value, current_group_size));
		sds_free(map_key);

#if MACHINE_ROLE_DEBUG
	log_write(LOG_DEBUG, "put ip = %s in map", map_key);
#endif

		while(--current_group_size)
		{
			get_line_param(fp, buf, key, value);
			map_key = sds_new(value);

#if MACHINE_ROLE_DEBUG
			log_write(LOG_DEBUG, "put ip = %s in map", map_key);
#endif
			allocator->roles->op->put(allocator->roles, map_key, create_data_server_role(current_master, value));
			sds_free(map_key);
		}
	}

	fclose(fp);
}


static void print_role(map_role_value_t *role){
	log_write(LOG_DEBUG, "role ip = %s", role->ip);
	log_write(LOG_DEBUG, "role rank = %d", role->rank);
}

/**
 * distribute the machine role
 */
static void allocate_machine_role(machine_role_allocator_t *allocator) {
	map_t *roles = local_allocator->roles;


	map_iterator_t *iter = create_map_iterator(roles);
	map_role_value_t *role;
	map_role_value_t *master_role;

#if MACHINE_ROLE_DEBUG
	log_write(LOG_DEBUG, "start allocate_machine_role and map size = %d", local_allocator->roles->current_size);
#endif

	char ip[16] = {};
	get_visual_ip(allocator->net_name, ip);
	printf("ZERO IP = %s\n", ip);
	sds t_ip = sds_new(ip);
	role = (map_role_value_t *)(roles->op->get(roles, t_ip));
	puts(role->ip);
	sds_free(t_ip);
	t_ip = sds_new(role->master_ip);
	master_role = (map_role_value_t *)(roles->op->get(roles, t_ip));
	role->rank = 0;
	role->master_rank = master_role->rank;
	sds_free(t_ip);
	map_role_value_t *zero_role = zmalloc(sizeof(*role));
	memcpy(zero_role, role, sizeof(*role));
	local_allocator->role = zero_role;

	role = roles->op->get(roles, sds_new("10.1.1.70"));
	printf("10.1.1.70 id is %d and ip is %s\n", role->rank, role->ip);

	role = roles->op->get(roles, sds_new("10.1.1.75"));
	printf("10.1.1.75 id is %d and ip is %s\n", role->rank, role->ip);
	while(iter->op->has_next(iter))
	{

		role = iter->op->next(iter);
		printf("--====%d---0-=\n", role->rank);

#if MACHINE_ROLE_DEBUG
		print_role(role);
#endif

		if(role->rank == 0){
			continue;
		}
		sds master_ip = sds_new(role->master_ip);
		master_role = (map_role_value_t *)(roles->op->get(roles, master_ip));
		role->master_rank = master_role->rank;

#if MACHINE_ROLE_DEBUG
		log_write(LOG_DEBUG, "start send role result and target rank is %d, ip is %s", role->rank, role->ip);
#endif

		allocator->server->op->send_result(role, role->rank, MACHINE_ROLE_GET_ROLE, sizeof(*role), ANS);

		sds_free(master_ip);
	}

	allocator->role = get_role(allocator->rank, allocator->net_name);
	destroy_map_iterator(iter);
	destroy_machine_role_allocater(allocator);

#if MACHINE_ROLE_DEBUG
	log_write(LOG_DEBUG, "end allocate_machine_role");
#endif
}

static void *get_event_handler_param(event_handler_t *event_handler) {
	return event_handler->special_struct;
}

/**
 * get machine role
 */
static void machine_register(event_handler_t *event_handler) {
	machine_register_role_t *param = get_event_handler_param(event_handler);

#if MACHINE_ROLE_DEBUG
	log_write(LOG_DEBUG, "register information source = %d, param ip = %s\n", param->source, param->ip);
#endif
	sds key_ip = sds_new(param->ip);
#if MACHINE_ROLE_DEBUG
	log_write(LOG_DEBUG, "machine register ip is %s and id = %d", param->ip, param->source);
#endif

#if MACHINE_ROLE_DEBUG
	log_write(LOG_DEBUG, "key ip  = %s param ip = %s", key_ip, param->ip);
#endif
	map_role_value_t *role = local_allocator->roles->op->get(local_allocator->roles, key_ip);

#if MACHINE_ROLE_DEBUG
	log_write(LOG_DEBUG, "machine register ip is %d and id = %s", param->source, param->ip);
#endif

	role->rank = param->source; //copy role rank

	pthread_mutex_lock(local_allocator->mutex_allocator);
	local_allocator->register_machine_num++;
	pthread_mutex_unlock(local_allocator->mutex_allocator);

#if MACHINE_ROLE_DEBUG
	log_write(LOG_DEBUG, "register_machine_num = %d", local_allocator->register_machine_num);
#endif
	sds_free(key_ip);
	zfree(CMD_TO_COMM_MSG(param));
	if(local_allocator->register_machine_num == local_allocator->machine_num)
	{
		allocate_machine_role(local_allocator);
	}
}

static void *resolve_handler(event_handler_t *event_handler, void* msg_queue) 
{
	common_msg_t *common_msg = zmalloc(sizeof(*common_msg));
	syn_queue_t* queue = msg_queue;
	queue->op->syn_queue_pop(queue, common_msg);
	switch(common_msg->operation_code)
	{
		case MACHINE_REGISTER_TO_MASTER:
			event_handler->special_struct = MSG_COMM_TO_CMD(common_msg);
			event_handler->handler = machine_register;
			break;
		default:
			event_handler->handler = NULL;
	}
	return event_handler->handler;
}

/**
 * create allocator
 */
static machine_role_allocator_t *create_machine_role_allocator(size_t size, int rank, char *file_path, char *net_name) {
	machine_role_allocator_t *this = zmalloc(sizeof(machine_role_allocator_t));
	strcpy(this->file_path, file_path);
	local_allocator = this;
	this->rank = 0;
	this->machine_num = size;
	this->net_name = net_name;
	this->register_machine_num = 0;
	this->roles = create_map(size * 2, NULL, NULL, NULL, map_list_pair_free);
	this->op = zmalloc(sizeof(machine_role_allocator_op_t));

	this->op->get_net_topology = get_net_topology;
	this->op->machine_register = machine_register;
	this->server = create_rpc_server(1, 1024, rank, resolve_handler);

	this->mutex_allocator = zmalloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(this->mutex_allocator, NULL);

	return this;
}

void send_server_stop_cmd(rpc_server_t *server)
{
	rpc_client_t *client = create_rpc_client(server->server_id, server->server_id, 781);
	stop_server_msg_t* stop_server_msg = NULL;
	stop_server_msg = zmalloc(sizeof(stop_server_msg_t));
	stop_server_msg->operation_code = SERVER_STOP;
	stop_server_msg->source = server->server_id;
	stop_server_msg->tag = 781;
	client->op->set_send_buff(client, stop_server_msg, sizeof(stop_server_msg_t));
	if(client->op->execute(client, STOP_SERVER) < 0)
	{
		log_write(LOG_TRACE, "something wrong\n");
	}
	else
	{
		log_write(LOG_DEBUG, "GET ROLE STOP SERVER!!!\n");
	}
	destroy_rpc_client(client);
}

static void destroy_machine_role_allocater(machine_role_allocator_t *this) {
#if MACHINE_ROLE_DEBUG
	log_write(LOG_DEBUG, "destroy_machine_role_allocater and map size is %d", local_allocator->roles->current_size);
#endif

	destroy_map(this->roles);

#if MACHINE_ROLE_DEBUG
	log_write(LOG_DEBUG, "destroy map");
#endif

	send_server_stop_cmd(this->server);

#if MACHINE_ROLE_DEBUG
	log_write(LOG_DEBUG, "send_server_stop_cmd");
#endif

	destroy_rpc_server(this->server);

#if MACHINE_ROLE_DEBUG
	log_write(LOG_DEBUG, "destroy role allocator");
#endif

	pthread_mutex_destroy(this->mutex_allocator);
	zfree(this->op);
	zfree(this);
}

map_role_value_t *machine_role_allocator_start(size_t size, int rank, char *file_path, char *net_name){
#if MACHINE_ROLE_DEBUG
	log_write(LOG_DEBUG, "start create role allocator");
#endif
	machine_role_allocator_t *allocator = create_machine_role_allocator(size, rank, file_path, net_name);
#if MACHINE_ROLE_DEBUG
	log_write(LOG_DEBUG, "create role allocator success");
#endif
	allocator->op->get_net_topology(allocator);
#if MACHINE_ROLE_DEBUG
	log_write(LOG_DEBUG, "get net topology success");
#endif
	allocator->server->op->server_start(allocator->server);
#if MACHINE_ROLE_DEBUG
	log_write(LOG_DEBUG, "server start success");
#endif
	return allocator->role;
}

/*------------------Machine Role Fetcher--------------------*/
//TODO get machine visual ip
void get_visual_ip(const char *net_name, char *ip){
	struct ifaddrs * ifAddrStruct = NULL;
		void * tmpAddrPtr = NULL;

		getifaddrs(&ifAddrStruct);
		while (ifAddrStruct != NULL) {
			if (ifAddrStruct->ifa_addr->sa_family == AF_INET) { // check it is IP4
				// is a valid IP4 Address
				tmpAddrPtr =
						&((struct sockaddr_in *) ifAddrStruct->ifa_addr)->sin_addr;
				inet_ntop(AF_INET, tmpAddrPtr, ip, INET_ADDRSTRLEN);
				if(strcmp(ifAddrStruct->ifa_name, net_name) == 0){
					return;
				}
			} else if (ifAddrStruct->ifa_addr->sa_family == AF_INET6) { // check it is IP6
				// is a valid IP6 Address
				tmpAddrPtr =
						&((struct sockaddr_in *) ifAddrStruct->ifa_addr)->sin_addr;
				inet_ntop(AF_INET6, tmpAddrPtr, ip, INET6_ADDRSTRLEN);
				if(strcmp(ifAddrStruct->ifa_name, net_name) == 0){
					return;
				}
			}
			ifAddrStruct = ifAddrStruct->ifa_next;
		}
}

static map_role_value_t* register_to_zero_rank(struct machine_role_fetcher *fetcher){
	machine_register_role_t *role = zmalloc(sizeof(*role));
	role->source = fetcher->rank;
	get_visual_ip(fetcher->net_name, role->ip);

#if MACHINE_ROLE_FETCHER_DEBUG
	log_write(LOG_DEBUG, "get_visual_ip = %s\n", role->ip);
#endif

	role->operation_code = MACHINE_REGISTER_TO_MASTER;
	role->unique_tag = MACHINE_ROLE_GET_ROLE;
	fetcher->client->op->set_send_buff(fetcher->client, role, sizeof(*role));

#if MACHINE_ROLE_FETCHER_DEBUG
	log_write(LOG_DEBUG, "set_send_buff");
#endif

	int result = fetcher->client->op->execute(fetcher->client, COMMAND_WITH_RETURN);

#if MACHINE_ROLE_FETCHER_DEBUG
	log_write(LOG_DEBUG, "fetch send request");
#endif

	zfree(role);
	return fetcher->client->recv_buff;
}

static machine_role_fetcher_t *create_machine_role_fetcher(int rank, char *net_name){
	machine_role_fetcher_t *fetcher = zmalloc(sizeof(*fetcher));
	fetcher->rank = rank;
	fetcher->client = create_rpc_client(rank, 0, MACHINE_ROLE_GET_ROLE);
	fetcher->op = zmalloc(sizeof(*fetcher->op));
	fetcher->op->register_to_zero_rank = register_to_zero_rank;
	strcpy(fetcher->net_name, net_name);

	return fetcher;
}

static void destroy_machine_role_fetcher(machine_role_fetcher_t *fetcher)
{
	destroy_rpc_client(fetcher->client);
	zfree(fetcher->op);
}

map_role_value_t *get_role(int rank, char *net_name){
	if(rank == 0)
	{
		while(local_allocator->role == NULL);
		return local_allocator->role;
	}

#if MACHINE_ROLE_FETCHER_DEBUG
	log_write(LOG_DEBUG, "create_machine_role_fetcher");
#endif
	machine_role_fetcher_t *fetcher = create_machine_role_fetcher(rank, net_name);
#if MACHINE_ROLE_FETCHER_DEBUG
	log_write(LOG_DEBUG, "register_to_zero_rank");
#endif
	map_role_value_t *role = fetcher->op->register_to_zero_rank(fetcher);
#if MACHINE_ROLE_FETCHER_DEBUG
	log_write(LOG_DEBUG, "destroy_machine_role_fetcher");
#endif
	destroy_machine_role_fetcher(fetcher);
	return role;
}
