/*
 * master.c
 *
 *  Created on: 2015年7月8日
 *      Author: ron
 */

#include <stdio.h>
#include "./common/zmalloc.h"
#include "./common/threadpool.h"
#include "./common/syn_tool.h"
#include "master.h"

/*--------------------Private Declaration--------------------*/
static master_t *local_master;

/**
 * master_server handle request
 *
 * machine register
 * create persistent file
 * delete file in the file system
 * sub-master heart blood
 * sub-master ask for global id
 * ask for persisting file to the disk
 * finish appending file
 */
/*---------------Master Service Start---------------*/
static uint64_t static_get_global_id(master_t *master, uint64_t num) {
	assert(num > 0);

	pthread_mutex_lock(&(master->mutex_global_id));
	uint64_t result = master->global_id;
	master->global_id += num;
	pthread_mutex_unlock(&(master->mutex_global_id));

	return result;
}

static void *get_event_handler_param(event_handler_t *event_handler) {
	return event_handler->event_buffer_list->head->value;
}

static void create_persistent_file(event_handler_t *event_handler) {
	master_create_file_t *create_file = get_event_handler_param(event_handler);
	master_create_file_ans_t *result = zmalloc(sizoef(*result));
	sds name = sds_new(create_file->name);
	result->result_code = local_master->name_space->op->add_temporary_file(local_master->name_space, name);
	local_master->rpc_server->op->send_result(result, create_file->source, create_file->tag, sizeof(*result));
	sds_free(name);
	zfree(result);
}

static void allocate_append_space() {

}

/*
 * TODO
 * check if the file is totally consistent to the disk
 * if so delete
 * else consistent it and delete
 */
static void delete_system_file(event_handler_t *event_handler) {
	master_deleter_system_file_t *delete_file = get_event_handler_param(event_handler);
	master_deleter_system_file_ans_t *result = zmalloc(sizoef(*result));
	result->result_code = local_master->name_space->op->delete_file(local_master->name_space, delete_file->name);
	local_master->rpc_server->op->send_result(result, delete_file->source, delete_file->tag, sizeof(*result));
	zfree(result);
}

static void sub_master_heart_blood(event_handler_t *event_handler) {

}

static void sub_master_ask_global_id(event_handler_t *event_handler) {
	sub_master_ask_global_id_t *ask = get_event_handler_param(event_handler);
	master_global_id_ans_t *result = zmalloc(sizeof(*result));
	result->start = static_get_global_id(local_master, ask->id_num);
	result->end = result->start + ask->id_num;
	local_master->rpc_server->op->send_result(result, ask->source, ask->tag, sizeof(*result));
	zfree(result);
}

static void consistent_file_to_disk(event_handler_t *event_handler) {
	//master_append_file_ans_t *append = get_event_handler_param(event_handler);

}

static void append_file(event_handler_t *event_handler) {
	master_append_file_t *t = get_event_handler_param(event_handler);
	master_append_file_ans_t *result = zmalloc(sizeof(*result));
	result->result_code = local_master->name_space->op->append_file(local_master->name_space, t->file_name, t->append_size);

}

static void *resolve_handler(event_handler_t* event_handler, void* msg_queue) {
	common_msg_t common_msg;
	syn_queue_t* queue = msg_queue;
	queue->op->syn_queue_pop(queue, &common_msg);
	switch(common_msg.operation_code)
	{
		case MASTER_CREATE_PERSISTENT_FILE:
			event_handler->handler = create_persistent_file;
			break;
		default:
			event_handler->handler = NULL;
	}
	return event_handler->handler;
}

/*---------------Master Service End---------------*/

/*
 * allocate necessary space
 * get net work topology from configure file
 * parse machine group
 * receive register information from other machines
 *
 */
static void init() {

}
/**
 * handle file persistence
 *
 */
static void handle_persistent() {

}


master_t *create_master(size_t size, int rank, int machine_num) {
	master_t *master = zmalloc(sizeof(master_t));

	local_master = master;
	master->name_space = create_name_space(1024);
	master->comm = MPI_COMM_WORLD;
	master->rank = rank;
	master->thread = zmalloc(sizeof(pthread_t) * 4);
	master->rpc_server = create_mpi_rpc_server(4, rank, resolve_handler);

	return master;
}

void destroy_master(master_t *master) {

}



