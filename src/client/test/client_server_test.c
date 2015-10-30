#include "dataserver.h"
#include "zmalloc.h"
#include "log.h"
#include "client_server.h"
#include "mpi_communication.h"

static zserver_t *zserver;
static rpc_server_t *local_server;

void create_file_handler(event_handler_t *event_handler)
{
	client_create_file_t *client_create_file;
	file_sim_ret_t *file_sim_ret;

	log_write(LOG_DEBUG, "create file handler stub");
	client_create_file = event_handler->special_struct;
	log_write(LOG_INFO, "file name is %s", client_create_file->file_name);
	log_write(LOG_INFO, "unique tag is %d", client_create_file->unique_tag);
	log_write(LOG_INFO, "file mode is %o", client_create_file->file_mode);
	file_sim_ret = zmalloc(sizeof(file_sim_ret_t));
	file_sim_ret->op_status = ACC_OK;
	local_server->op->send_result(file_sim_ret, 1, 13, sizeof(file_sim_ret_t),
			ANS);
	zfree(file_sim_ret);
}

void append_file_handler(event_handler_t *event_handler)
{
	client_append_file_t *client_append_file;
	file_ret_t *file_ret;

	log_write(LOG_DEBUG, "append file handler stub");
	client_append_file = event_handler->special_struct;
	log_write(LOG_INFO, "file name is %s", client_append_file->file_name);
	log_write(LOG_INFO, "unique tag is %d", client_append_file->unique_tag);
	file_ret = zmalloc(sizeof(file_ret_t));
	file_ret->op_status = ACC_OK;
	file_ret->file_size = 21;
	file_ret->offset = 0;
	file_ret->dataserver_num = 1;
	file_ret->chunks_num = 1;
	file_ret->data_server_arr[0] = 2;
	file_ret->data_server_cnum[0] = 1;
	file_ret->data_server_offset[0] = 0;
	file_ret->data_server_len[0] = 20;
	file_ret->chunks_id_arr[0] = 0;
	local_server->op->send_result(file_ret, 1, 13, sizeof(file_ret_t),
			ANS);
	zfree(file_ret);
}

void *resolve_handler(event_handler_t *event_handler, void *msg_queue)
{
	common_msg_t common_msg;

	syn_queue_t *queue = msg_queue;
	queue->op->syn_queue_pop(queue, &common_msg);

	while(common_msg.operation_code / 1000 == 5)
	{
		zserver->op->zput_request(zserver, &common_msg);
		queue->op->syn_queue_pop(queue, &common_msg);
	}
	switch(common_msg.operation_code)
	{
		case SERVER_STOP:
			init_server_stop_handler(event_handler, local_server,
					&common_msg);
			event_handler->handler = server_stop_handler;
			break;

		case CREATE_TEMP_FILE_CODE:
			event_handler->handler = create_file_handler;
			event_handler->special_struct = MSG_COMM_TO_CMD(&common_msg);
			break;
			
		case APPEND_FILE_CODE:
			event_handler->handler = append_file_handler;
			event_handler->special_struct = MSG_COMM_TO_CMD(&common_msg);
			break;

		default:
			event_handler->handler = NULL;
			break;
	}
	return event_handler->handler;
}

int main(int argc, char *argv[])
{
	int id;

	log_init("", LOG_DEBUG);
	mpi_init_with_thread(&argc, &argv);
	id = get_mpi_rank();
	if(!id)
	{
		rpc_server_t *rpc_server;

		rpc_server = create_rpc_server(1, 8, 0, resolve_handler);
		local_server = rpc_server;
		zserver = create_zserver(id);
		zserver->op->zserver_start(zserver);
		rpc_server->op->server_start(rpc_server);

		destroy_rpc_server(rpc_server);
		zserver->op->zserver_stop(zserver);
		destroy_zserver(zserver);
	}
	else if(id == 1)
	{
		fclient_t *fclient;

		fclient = create_fclient(id, 0, 13);
		fclient_run(fclient);
		destroy_fclient(fclient);
	}
	else
	{
		data_server_t *dataserver;

		dataserver = alloc_dataserver(MIDDLE, id);
		dataserver_run(dataserver);
		destroy_dataserver(dataserver);
	}
	mpi_finish();
	return 0;
}
