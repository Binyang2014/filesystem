#include "dataserver.h"
#include "log.h"
#include "client_server.h"
#include "mpi_communication.h"

static zserver_t *zserver;
static rpc_server_t *local_server;

void create_file_handler(event_handler_t *event_handler)
{
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
			break;

		default:
			event_handler->handler = NULL;
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
