/*
 * data_master.c
 *
 *  Created on: 2015年10月8日
 *      Author: ron
 */

#include <math.h>
#include <string.h>
#include <assert.h>
#include "data_master.h"
#include "list_queue_util.h"
#include "zmalloc.h"
#include "log.h"

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
	return event_handler->special_struct;
}

static int has_enough_space(uint64_t block_num){
	if(block_num < local_master->free_size){
		return 1;
	}else{
		return 0;
	}
}

static void *position_dup(const void *ptr){
	position_des_t *position = zmalloc(sizeof(*position));
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
	uint64_t tmp_al_size = allocate_size;
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
		list->list_ops->list_add_node_tail(list, position);
	}

	if(allocate_size == 0){
		return list;
	}
	basic_queue_t *queue = local_master->storage_q->queue;
	int end = index;
	int t_index = index;
	storage_machine_sta_t *st;
	do{
		st = get_queue_element(queue, t_index);
		if(st->free_blocks){
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
			}
			list->list_ops->list_add_node_tail(list, position);
		}
		t_index = (t_index + 1) % queue->current_size;
	}while(t_index != end && allocate_size != 0);

	t_index = index;
	if(allocate_size == 0){
		local_master->free_size -= tmp_al_size;
		return list;
	}else{
		list_iter_t *iter = list->list_ops->list_get_iterator(list, AL_START_HEAD);
		while(list->list_ops->list_has_next(iter)){
			position_des_t *position = ((list_node_t *)list->list_ops->list_next(iter))->value;
			do{
				st = get_queue_element(queue, t_index++);
			}while(st->rank != position->rank);
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
	client_create_file_t *c_cmd = get_event_handler_param(event_handler);
	sds file_name = sds_new(c_cmd->file_name);

	pthread_mutex_lock(local_master->mutex_data_master);
#if DATA_MASTER_DEBUG
	log_write(LOG_TRACE, "create tmp file local_master = %d", local_master->namespace->op);
#endif
	assert(!local_master->namespace->op->file_exists(local_master->namespace, file_name));

#if DATA_MASTER_DEBUG
	log_write(LOG_TRACE, "create tmp file and file not exists");
#endif

	local_master->namespace->op->add_temporary_file(local_master->namespace, file_name);
	pthread_mutex_unlock(local_master->mutex_data_master);
#if DATA_MASTER_DEBUG
	log_write(LOG_TRACE, "create tmp file end");
#endif
	//send result
}

static void append_temp_file(event_handler_t *event_handler){

#if DATA_MASTER_DEBUG
	log_write(LOG_TRACE, "append tmp file start");
#endif

	client_append_file_t *c_cmd = get_event_handler_param(event_handler);
	sds file_name = sds_new(c_cmd->file_name);
	assert(c_cmd->write_size > 0);
	assert(local_master->namespace->op->file_exists(local_master->namespace, file_name));
	int index = hash_code(file_name, local_master->group_size);

#if DATA_MASTER_DEBUG
	log_write(LOG_TRACE, "data master append tmp file file exists");
#endif

	pthread_mutex_lock(local_master->mutex_data_master);
	list_t *list = allocate_space(c_cmd->write_size, index, local_master->namespace->op->get_file_node(local_master->namespace, file_name));

#if DATA_MASTER_DEBUG
	log_write(LOG_TRACE, "data master append allocate space end");
#endif

	assert(list != NULL);
	int size = list->len;
	void *pos_arrray = list_to_array(list, sizeof(position_des_t));
#if DATA_MASTER_DEBUG
	log_write(LOG_TRACE, "data master append list to array size = %d", size);
#endif

	//send write result to client
	local_master->rpc_server->op->send_result(pos_arrray, c_cmd->source, c_cmd->unique_tag, size * sizeof(position_des_t), ANS);

#if DATA_MASTER_DEBUG
	log_write(LOG_TRACE, "data master append send result");
#endif

	//set location
	local_master->namespace->op->set_file_location(local_master->namespace, file_name, list);

#if DATA_MASTER_DEBUG
	log_write(LOG_TRACE, "data master append set file location");
#endif

	local_master->namespace->op->append_file(local_master->namespace, file_name, c_cmd->write_size);

#if DATA_MASTER_DEBUG
	log_write(LOG_TRACE, "data master append name space append file");
#endif
	pthread_mutex_unlock(local_master->mutex_data_master);

	zfree(pos_arrray);
#if DATA_MASTER_DEBUG
	log_write(LOG_TRACE, "append tmp file end");
#endif
}

static list_t* get_file_list_location(uint64_t read_blocks, uint64_t read_offset, list_t *list)
{
	list_iter_t *iter = list->list_ops->list_get_iterator(list, AL_START_HEAD);
	list_t *result = list_create();
	result->free = position_free;
	result->dup = position_dup;
	position_des_t *position;
	position_des_t *list_p;

	int count = 0; //how many blocks this position can read
	log_write(LOG_DEBUG, "read_blocks = %d, read_offset = %d, list = %d", read_blocks, read_offset, ((position_des_t *)(list->head)));
	while(read_blocks > 0)
	{
		position = (position_des_t *)((list_node_t *)list->list_ops->list_next(iter)->value);
		count = position->end - position->start + 1;
		list_p = zmalloc(sizeof(*list_p));
		if(read_offset > 0)
		{
			if(read_offset >= count)
			{
				read_offset -= count;
				continue;
			}else
			{
				list_p->start = position->start + read_offset;
				if(read_blocks +  read_offset <= count)
				{
					list_p->rank = position->rank;
					list_p->end = position->start + read_blocks + read_offset - 1;
					read_blocks = 0;
				}
				else
				{
					list_p->end = position->end;
					read_blocks -= count;
				}
				read_offset = 0;
			}
		}
		else
		{
			if(read_blocks <= count)
			{
				list_p->rank = position->rank;
				list_p->start = position->start;
				list_p->end = position->start + read_blocks - 1;
				read_blocks = 0;
			}
			else
			{
				list_p = (position_des_t *)list->dup(position);
				read_blocks -= count;
			}
		}
		result->list_ops->list_add_node_tail(result, list_p);
	}
	return result;
}

static void read_temp_file(event_handler_t *event_handler){

#if DATA_MASTER_DEBUG
	log_write(LOG_TRACE, "read tmp file start");
#endif

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
	assert(read_size + offset <= node->file_size);
	uint64_t read_offset = offset / BLOCK_SIZE;
	if(offset % BLOCK_SIZE == 0){
		read_blocks = ceil((double)read_size / BLOCK_SIZE);
	}else{
		read_blocks = ceil((double)(read_size + offset) / BLOCK_SIZE) - offset / BLOCK_SIZE;
	}

#if DATA_MASTER_DEBUG
	log_write(LOG_TRACE, "read tmp file read_blocks = %d", read_blocks);
#endif

	list_t *result = get_file_list_location(read_blocks, read_offset, list);
#if DATA_MASTER_DEBUG
	log_write(LOG_TRACE, "read tmp file get_file_list_location");
#endif
	void *pos_arrray = list_to_array(result, sizeof(position_des_t));
#if DATA_MASTER_DEBUG
	log_write(LOG_TRACE, "read tmp file list_to_array");
#endif
	local_master->rpc_server->op->send_result(pos_arrray, c_cmd->source, c_cmd->unique_tag, list->len * sizeof(position_des_t), ANS);
#if DATA_MASTER_DEBUG
	log_write(LOG_TRACE, "read tmp file send_result");
#endif
	list_release(result);
#if DATA_MASTER_DEBUG
	log_write(LOG_TRACE, "read tmp file list_release");
#endif
	zfree(pos_arrray);

#if DATA_MASTER_DEBUG
	log_write(LOG_TRACE, "read tmp file end");
#endif
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
#if DATA_MASTER_DEBUG
	log_write(LOG_DEBUG, "start register rank = %d, ip = %s", cmd->source, cmd->ip);
#endif
	pthread_mutex_lock(local_master->mutex_data_master);
	local_master->free_size += stor->free_blocks;
	pthread_mutex_unlock(local_master->mutex_data_master);

	local_master->storage_q->op->syn_queue_push(local_master->storage_q, stor);

#if DATA_MASTER_DEBUG
	log_write(LOG_DEBUG, "end register and group size = %d", local_master->storage_q->queue->current_size);
#endif
	zfree(stor);
}

static void *resolve_handler(event_handler_t* event_handler, void* msg_queue) {
	common_msg_t common_msg;
	syn_queue_t* queue = msg_queue;
	queue->op->syn_queue_pop(queue, &common_msg);
	switch(common_msg.operation_code)
	{
		case REGISTER_TO_DATA_MASTER_CODE:
			event_handler->special_struct = MSG_COMM_TO_CMD(&common_msg);
			event_handler->handler = register_to_master;
			break;
		case CREATE_TEMP_FILE_CODE:
			event_handler->special_struct = MSG_COMM_TO_CMD(&common_msg);
			event_handler->handler = create_temp_file;
			break;
		case APPEND_TEMP_FILE_CODE:
			event_handler->special_struct = MSG_COMM_TO_CMD(&common_msg);
			event_handler->handler = append_temp_file;
			break;
		case READ_TEMP_FILE_CODE:
			event_handler->special_struct = MSG_COMM_TO_CMD(&common_msg);
			event_handler->handler = read_temp_file;
			break;
		case DELETE_TMP_FILE_CODE:
			event_handler->special_struct = MSG_COMM_TO_CMD(&common_msg);
			event_handler->handler = delete_temp_file;
			break;
		case SERVER_STOP:
			init_server_stop_handler(event_handler, local_master->rpc_server, &common_msg);
			event_handler->handler = server_stop_handler;
			break;
		default:
			event_handler->handler = NULL;
	}
	return event_handler->handler;
}


static storage_machine_sta_t* get_self_stor()
{
	storage_machine_sta_t *re = zmalloc(sizeof(*re));
	re->free_blocks = local_master->free_blocks;
	re->rank = local_master->rank;
	re->visual_ip = sds_new(local_master->visual_ip);
	return re;
}
/**
 * TODO
 * start server
 *
 */
void* data_master_init(void *args){
	data_master_t *master = args;
	storage_machine_sta_t *re = get_self_stor();
	local_master->storage_q->op->syn_queue_push(local_master->storage_q, re);
	master->rpc_server->op->server_start(master->rpc_server);
	return 0;
}

/**
 *
 */
data_master_t* create_data_master(map_role_value_t *role, uint64_t free_blocks){
	data_master_t *this = zmalloc(sizeof(*this));
	local_master = this;

	this->group_size =  role->group_size;
	this->namespace = create_name_space(1024);
	//TODO thread num must be 1
	this->rpc_server = create_rpc_server(1, 1024, role->rank, resolve_handler);
	this->storage_q = alloc_syn_queue(role->group_size, sizeof(storage_machine_sta_t));
	this->rank = role->rank;
	this->free_size = 0;
	this->free_blocks = free_blocks;
	this->visual_ip = sds_new(role->ip);
	this->master_rank = role->master_rank;
	//this->global_id

	this->mutex_data_master = zmalloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(this->mutex_data_master, NULL);
	return this;
}

void destroy_data_master(data_master_t *this){
	this->rpc_server->op->server_stop(this->rpc_server);
	destroy_name_space(this->namespace);
	zfree(this->storage_q);
	sds_free(this->visual_ip);
}


