/*
 * data_master.c
 *
 *  Created on: 2015年10月8日
 *      Author: ron
 */

#include <math.h>
#include "data_master.h"
#include "../common/structure_tool/list_queue_util.h"

/*--------------------Private Declaration------------------*/
static int has_enough_space();
static list_t *allocate_space();
static void create_temp_file();
static void append_temp_file();
static void read_temp_file();
static void delete_temp_file();

static data_master_t *local_master;
/*--------------------API Declaration----------------------*/


/*--------------------Private Implementation---------------*/

static uint32_t hash_code(const sds key, size_t size) {
	size_t len = sds_len(key);
	/* 'm' and 'r' are mixing constants generated offline.
	   They're not really 'magic', they just happen to work well.  */
	uint32_t seed = 5381;
	const uint32_t m = 0x5bd1e995;
	const int r = 24;

	/* Initialize the hash to a 'random' value */
	uint32_t h = seed ^ len;

	/* Mix 4 bytes at a time into the hash */
	const unsigned char *data = (const unsigned char *)key;
	while(len >= 4) {
		uint32_t k = *(uint32_t*)data;
		k *= m;
		k ^= k >> r;
		k *= m;

		h *= m;
		h ^= k;

		data += 4;
		len -= 4;
	}
	/* Handle the last few bytes of the input array  */
	switch(len) {
		case 3: h ^= data[2] << 16;
				h ^= data[1] << 8;
				h ^= data[0]; h *= m;
				break;
		case 2: h ^= data[1] << 8;
				h ^= data[0]; h *= m;
				break;
		case 1: h ^= data[0]; h *= m;
	};

	/* Do a few final mixes of the hash to ensure the last few
	 * bytes are well-incorporated. */
	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;

	return (uint32_t)h % size;
}

/**
 * Thread pool handler parameter
 */
static void *get_event_handler_param(event_handler_t *event_handler) {
	return event_handler->event_buffer_list->head->value;
}

static int has_enough_space(uint64_t block_num){
	if(block_num < local_master->free_size){
		return 0;
	}else{
		return 1;
	}
}

static void *position_dup(const void *ptr){
	position_des_t *position = zmalloc(sizeof(position_des_t));
	memcpy(position, ptr, sizeof(*position));
	return position;
}

static void position_free(void *ptr){
	zfree(ptr);
}

/**
 * allocate file space
 * first judge whether there is enough space, if not break the process
 */
static list_t *allocate_space(uint64_t write_size, int index, file_node_t *node){
	uint64_t allocate_size;
	if((node->file_size) % BLOCK_SIZE == 0){
		allocate_size = ceil((double)write_size / BLOCK_SIZE);
	}else{
		allocate_size = ceil((double)(write_size + node->file_size) / BLOCK_SIZE) - ceil((double)(node->file_size) / BLOCK_SIZE);
	}

	assert(has_enough_space(allocate_size));

	uint64_t start_global_id = local_master->global_id;
	uint64_t end_global_id = start_global_id + allocate_size;
	local_master->global_id = end_global_id;

	list_t *list = list_create();
	list->free = position_free;
	list->dup = position_dup;
	position_des_t *position = zmalloc(sizeof(position_des_t));
	if((node->file_size) % BLOCK_SIZE != 0){
		memcpy(position, node->position->tail->value, sizeof(position_des_t));
		position->start = position->end;
	}

	if(allocate_size == 0){
		return list;
	}
	basic_queue_t *queue = local_master->storage_q;
	int end = index;
	int t_index = index;
	storage_machine_sta_t *st;
	do{
		st = get_queue_element(queue, t_index);
		if(!st->free_blocks){
			continue;
		}
		position = zmalloc(sizeof(position_des_t));
		position->rank = st->rank;
		position->start = start_global_id;
		if(st->free_blocks < allocate_size){
			position->end = start_global_id + st->free_blocks - 1;
			start_global_id = position->end + 1;

			allocate_size -= st->free_blocks;
			st->used_blocks += st->free_blocks;
			st->free_blocks = 0;
		}else{
			position->end = start_global_id + allocate_size - 1;
			start_global_id = position->end + 1;

			st->free_blocks -= allocate_size;
			st->used_blocks += allocate_size;
			allocate_size = 0;
			break;
		}
		t_index = (t_index + 1) % queue->current_size;
		list->list_ops->list_add_node_tail(list, position);
	}while(t_index != end && allocate_size != 0);

	t_index = index;
	if(allocate_size == 0){
		return list;
	}else{
		list_iter_t *iter = list->list_ops->list_get_iterator(list, AL_START_HEAD);
		while(list->list_ops->list_has_next(iter)){
			position_des_t *position = ((list_node_t *)list->list_ops->list_next(iter))->value;
			do{
				st = get_queue_element(queue, t_index++);
			}while(st->rank != position.rank);
			int allocated_c = position->end - position->start + 1;
			st->free_blocks += allocated_c;
			st->used_blocks -= allocated_c;
		}
		list->list_ops->list_release_iterator(iter);
		list_release(list);
		return NULL;
	}
}

static void create_temp_file(event_handler_t *event_handler){
	//assert(exists);

	client_create_file *c_cmd = get_event_handler_param(event_handler);
	sds file_name = sds_new(c_cmd->file_name);

	pthread_mutex_lock(local_master->mutex_data_master);
	assert(!local_master->namespace->op->file_exists(local_master->namespace, file_name));
	local_master->namespace->op->add_temporary_file(file_name);
	pthread_mutex_unlock(local_master->mutex_data_master);
	//send result
}

static void append_temp_file(event_handler_t *event_handler){

	client_append_file_t *c_cmd = get_event_handler_param(event_handler);
	sds file_name = sds_new(c_cmd->file_name);
	assert(c_cmd->write_size > 0);
	assert(local_master->namespace->op->file_exists(local_master->namespace, file_name));
	int index = hash_code(file_name, local_master->group_size);

	pthread_mutex_lock(local_master->mutex_data_master);
	list_t *list = allocate_space(c_cmd->write_size, index, local_master->namespace->op->get_file_node(local_master->namespace, file_name));
	assert(list != NULL);
	int size = list->len;
	void *pos_arrray = list_to_array(list, sizeof(position_des_t));

	//send write result to client
	local_master->rpc_server->op->send_result(pos_arrray, c_cmd->source, c_cmd->tag, size * sizeof(position_des_t), ANS);

	//set location
	local_master->namespace->op->set_file_location(local_master->namespace, file_name, list);
	local_master->namespace->op->append_file(local_master->namespace, file_name, c_cmd->write_size);
	pthread_mutex_unlock(local_master->mutex_data_master);

	list_release(list);
	zfree(pos_arrray);
}

static list_t* get_file_location(uint64_t read_blocks, uint64_t read_offset, list *queue){

}

static void read_temp_file(event_handler_t *event_handler){

	client_read_file_t *c_cmd = get_event_handler_param(event_handler);
	sds file_name = sds_new(c_cmd->file_name);

	assert(local_master->namespace->op->file_exists(local_master->namespace, file_name));

	pthread_mutex_lock(local_master->mutex_data_master);
	list_t *list = local_master->namespace->op->get_file_location(local_master->namespace, file_name);
	pthread_mutex_unlock(local_master->mutex_data_master);

	file_node_t *node = local_master->namespace->op->get_file_node(local_master->namespace, file_name);

	uint64_t read_size = c_cmd->read_size;
	uint64_t offset = c_cmd->offset;
	uint64_t read_blocks;
	uint64_t read_index;
	assert(read_size + offset <= node->file_size);
	read_index = offset / BLOCK_SIZE + 1;
	if(offset % BLOCK_SIZE == 0){
		read_blocks = ceil((double)read_size / BLOCK_SIZE);
	}else{
		read_blocks = ceil((double)(read_size + offset) / BLOCK_SIZE) - ceil((double)(offset) / BLOCK_SIZE) + 1;
	}

	//get file position
	//send result to client
}

static void delete_temp_file(event_handler_t *event_handler){
	//well this may be much more difficult
	//delete file node info and free space
}

static void register_to_master(event_handler_t *event_handler){
	c_d_register_t *cmd = get_event_handler_param(event_handler);
	storage_machine_sta_t *stor = zmalloc(sizeof(*stor));
	stor->rank = cmd->source;
	stor->free_blocks = cmd->free_block;
	stor->used_blocks = 0;
	stor->visual_ip = sds_new(cmd->ip);
	local_master->storage_q->basic_queue_op->push(local_master->storage_q, stor);
	zfree(stor);
}

static void *resolve_handler(event_handler_t* event_handler, void* msg_queue) {
	common_msg_t common_msg;
	syn_queue_t* queue = msg_queue;
	queue->op->syn_queue_pop(queue, &common_msg);
	switch(common_msg.operation_code)
	{
		case REGISTER_TO_DATA_MASTER:
			event_handler->handler = register_to_master;
			break;
		case CREATE_TEMP_FILE_CODE:
			event_handler->handler = create_temp_file;
			break;
		case APPEND_TEMP_FILE_CODE:
			event_handler->handler = append_temp_file;
			break;
		case READ_TEMP_FILE_CODE:
			event_handler->handler = read_temp_file;
			break;
		case DELETE_TMP_FILE_CODE:
			event_handler->handler = delete_temp_file;
			break;
		default:
			event_handler->handler = NULL;
	}
	return event_handler->handler;
}

/**
 * TODO
 * start server
 *
 */
void data_master_init(data_master_t *master){
	master->rpc_server->op->server_start(master->rpc_server);
	//initialize client
	//initialize data server
}

/**
 *
 */
data_master_t* create_data_master(map_role_value_t *role){
	data_master_t *this = zmalloc(sizeof(*this));

	this->group_size =  role->group_size;
	this->namespace = create_name_space(1024);
	this->rpc_server = create_rpc_server(8, 1024, 1, resolve_handler);
	this->storage_q = zmalloc(role->group_size * sizeof(storage_machine_sta_t));
	memset(this->storage_q, 0, role->group_size * sizeof(storage_machine_sta_t));
	this->rank = role->rank;
	this->visual_ip = sds_new(role->ip);
	this->master_rank = role->master_rank;
	//this->global_id

	this->mutex_data_master = zmalloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(this->mutex_data_master);
	return this;
}

void destroy_data_master(data_master_t *this){
	this->rpc_server->op->server_stop(this->rpc_server);
	destroy_name_space(this->namespace);
	zfree(this->storage_q);
	sds_free(this->visual_ip);
}


