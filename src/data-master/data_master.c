/*
 * data_master.c
 *
 *  Created on: 2015年10月8日
 *      Author: ron
 */

#include "data_master.h"

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

static list_t *allocate_space(uint64_t allocate_size){
	assert(has_enough_space(allocate_size));

	uint64_t start_global_id = local_master->global_id;
	uint64_t end_global_id = start_global_id + allocate_size;

	list_t *list = list_create();
	basic_queue_t *queue = local_master->storage_q;
	int index = 0;//TODO hash
	int end = index;

	do{
		storage_machine_sta_t *st = get_queue_element(queue, index);
		if(st->free_blocks < allocate_size){
				allocate_size -= st->free_blocks;
				st->used_blocks += st->free_blocks;
				st->free_blocks = 0;
				//TODO
		}else{
			st->free_blocks -= allocate_size;
			st->used_blocks += allocate_size;
			allocate_size = 0;
			break;
		}
	}while(index != end);

	if(allocate_size == 0){
		return list;
	}else{
		//TODO roll back
	}
}

static void create_temp_file(event_handler_t *event_handler){
	//assert(exists);
	c_d_create_t *c_cmd = get_event_handler_param(event_handler);
	sds file_name = sds_new(c_cmd->file_name);
	local_master->namespace->op->add_temporary_file(file_name);
	//send result
}

static void append_temp_file(event_handler_t *event_handler){

	//test whether file exists
	//calculate block size
	//allocate space
	//send position result to client
}

static void read_temp_file(event_handler_t *event_handler){
	//test whether file exists
	//get file position
	//send result to client
}

static void delete_temp_file(event_handler_t *event_handler){
	//well this may be much more difficult
}

static void *resolve_handler(event_handler_t* event_handler, void* msg_queue) {
	common_msg_t common_msg;
	syn_queue_t* queue = msg_queue;
	queue->op->syn_queue_pop(queue, &common_msg);
	switch(common_msg.operation_code)
	{
		case CREATE_TEMP_FILE_CODE:
			event_handler->handler = create_temp_file;
			break;
		case APPEND_TEMP_FILE_CODE:
			event_handler->handler = create_temp_file;
			break;
		case READ_TEMP_FILE_CODE:
			event_handler->handler = create_temp_file;
			break;
		default:
			event_handler->handler = NULL;
	}
	return event_handler->handler;
}

int data_master_init(){

}

int data_master_destroy(){

}

