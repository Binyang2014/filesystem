/*
 * sub_master.c
 *
 *  Created on: 2015年7月9日
 *      Author: ron
 */

#include "sub_master.h"

/*===============Private Declaration===============*/
static sub_master_t *local_this;

static void read_temp_file();
//static void

/*---------------------Sub-Master RPC Service------------------*/
static void *get_event_handler_param(event_handler_t *event_handler) {
	return event_handler->event_buffer_list->head->value;
}

static void allocate_temp_file_space(event_handler_t* event_handler) {

}

static void create_temp_file(event_handler_t* event_handler) {

}

static void delete_temp_file(event_handler_t* event_handler) {

}

static void append_tmp_file(event_handler_t* event_handler) {

}


/**
 * sub-master server
 *
 * receive data-master register information
 * receive data-master heart blood
 * create temporary file
 * read temporary file
 * delete temporary file
 * ask for allocate temporary file space
 * ask for allocate persistent file space
 */
static void sub_master_server() {

}

static void *resolve_handler(event_handler_t* event_handler, void* msg_queue) {
	common_msg_t common_msg;
	syn_queue_t* queue = msg_queue;
	queue->op->syn_queue_pop(queue, &common_msg);
	switch(common_msg.operation_code)
	{
		default:
			event_handler->handler = NULL;
	}
	return event_handler->handler;
}


sub_master_t *create_sub_master(int rank, machine_role_t *role) {
	sub_master_t *this = zmalloc(sizeof(*this));
	local_this = this;
	this->role = role;
	this->rpc_server = create_mpi_rpc_server(2, rank, resolve_handler);
	return this;
}
