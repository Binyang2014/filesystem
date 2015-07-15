/*
 * master.c
 *
 *  Created on: 2015年7月8日
 *      Author: ron
 */

#include <stdio.h>
#include "./common/zmalloc.h"
#include "./common/threadpool.h"
#include "./common/syn_queue.h"
#include "master.h"

/*---------------Private Declaration---------------*/
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
static int machine_register(event_handler_t *event_handler) {
	register_t *param = event_handler->event_buffer_list->head->value;
}

static int create_persistent_file(event_handler_t *event_handler) {
	//local_master->name_space-
}

static int delete_system_file(event_handler_t *event_handler) {

}

static void sub_master_heart_blood(event_handler_t *event_handler) {

}

static void sub_master_ask_global_id(event_handler_t *event_handler) {

}

static void consistent_file_to_disk(event_handler_t *event_handler) {

}

static void append_file(event_handler_t *event_handler) {

}

static void *resolve_handler(event_handler_t* event_handler, void* msg_queue) {
	common_msg_t common_msg;
	//syn_queue_t* queue = msg_queue;
	queue->op->syn_queue_pop(queue, &common_msg);
	switch(common_msg.operation_code)
	{

	}
	return event_handler->handler;
}

/*---------------Master Service End---------------*/

static void get_net_topology() {

}

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


master_t *create_master(size_t size, int rank) {
	master_t *master = zmalloc(sizeof(master_t));
	local_master = master;
	master->name_space = create_name_space(1024);
	master->comm = MPI_COMM_WORLD;
	master->rank = rank;
	master->thread = zmalloc(sizeof(pthread_t) * 4);
	master->rpc_server = create_mpi_rpc_server(4, rank, resolve_handler);
}

void destroy_master(master_t *master) {

}



