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
static int has_enough_space(uint64_t block_num){
	if(block_num < local_master->free_size){
		return 0;
	}else{
		return 1;
	}
}

static list_t *allocate_space(uint64_t allocate_size){
	assert(has_enough_space(allocate_size));

	list_t *list = list_create();
	list_t *iter_list = local_master->storage_machine_l;
	list_iter_t *iter = iter_list->list_ops->list_get_iterator(iter_list);
	while(iter_list->list_ops->list_has_next(iter)){
		storage_machine_sta_t *sta = iter_list->list_ops->list_next(iter);
	}
}

static void create_temp_file(event_handler_t *event_handler){

}

static void append_temp_file(event_handler_t *event_handler){

	//test whether file exists
	//
}

static void read_temp_file(event_handler_t *event_handler){

}

static void delete_temp_file(event_handler_t *event_handler){

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

