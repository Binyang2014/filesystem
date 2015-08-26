#include <stdio.h>
#include <stdlib.h>
#include "../rpc_client.h"
#include "../rpc_server.h"
#include "../message.h"
#include "../mpi_communication.h"
#include "../../structure_tool/zmalloc.h"
#include "../../structure_tool/log.h"
#include "../../structure_tool/threadpool.h"

static rpc_server_t *local_server;

typedef struct {
	uint16_t code;
	uint16_t transfer_version;
	int source;
	int tag;
	char rest[4084];
}test_rpc_t;

typedef struct {
	int result;
}test_result_t;

void hello(event_handler_t *event_handler) {
	puts("hello start");

	void *result = zmalloc(sizeof(test_result_t));
	((test_result_t *)(result))->result = 1090;
	local_server->op->send_result(result, 1, 1, sizeof(test_result_t), ANS);
	puts("hello end");
	zfree(result);
}

void *resolve_handler(event_handler_t *event_handler, void *msg_queue) {
	common_msg_t common_msg;

	syn_queue_t *queue = msg_queue;
	queue->op->syn_queue_pop(queue, &common_msg);
	switch(common_msg.operation_code)
	{
		case 1:
			puts("case 1");
			event_handler->handler = hello;
			break;

		case SERVER_STOP:
			init_server_stop_handler(event_handler, local_server, &common_msg);
			event_handler->handler = server_stop_handler;
			break;

		default:
			event_handler->handler = NULL;
	}
	return event_handler->handler;
}

int main(int argc, char* argv[])
{
	int rank;
	test_result_t* result;

	log_init("", LOG_DEBUG);
	mpi_init_with_thread(&argc, &argv);
	rank = get_mpi_rank();

	if(rank == 0) {
		rpc_server_t *server = create_rpc_server(3, 3, 0, resolve_handler);
		local_server = server;
		server->op->server_start(server);
		printf("Can you see me??\n");
		destroy_rpc_server(server);
	}else {
		rpc_client_t *client = create_rpc_client(rank, 0, 1);
		test_rpc_t *msg = zmalloc(4096);
		stop_server_msg_t* stop_server_msg = NULL;

		msg->code = 1;
		msg->source = 1;
		msg->tag = 1;
		client->op->set_send_buff(client, msg);
		if(client->op->execute(client, COMMAND_WITH_RETURN) < 0)
			printf("execute wrong\n");
		else
		{
			result = client->recv_buff;
			printf("rpc test result = %d\n", result->result);
		}
		zfree(msg);

		//send message to stop server
		stop_server_msg = zmalloc(sizeof(stop_server_msg_t));
		stop_server_msg->operation_code = SERVER_STOP;
		stop_server_msg->source = 1;
		stop_server_msg->tag = 1;
		client->op->set_send_buff(client, stop_server_msg);
		if(client->op->execute(client, STOP_SERVER) < 0)
			printf("something wrong\n");
		else
			printf("I can't believe it!!!\n");
		destroy_rpc_client(client);
		zfree(stop_server_msg);
	}
	mpi_finish();
	return 0;
}
