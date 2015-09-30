#include <stdio.h>
#include <unistd.h>
#include "zookeeper.h"
#include "zmalloc.h"
#include "log.h"
#include "mpi_communication.h"

static zserver_t *zserver;
static rpc_server_t *local_server;

void *watch_handler_change(void *args)
{
	printf("\n");
	printf("------------------------------\n");
	printf("This is a change watch handler\n");
	printf("------------------------------\n");
	printf("see me, see me, see me, see me\n");
	printf("------------------------------\n");
	printf("\n");
	return NULL;
}

void *watch_handler_delete(void *args)
{
	printf("\n");
	printf("------------------------------\n");
	printf("This is a delete watch handler\n");
	printf("------------------------------\n");
	printf("see me, see me, see me, see me\n");
	printf("------------------------------\n");
	printf("\n");
	return NULL;
}

void *resolve_handler(event_handler_t *event_handler, void *msg_queue)
{
	common_msg_t common_msg;

	syn_queue_t *queue = msg_queue;
	queue->op->syn_queue_pop(queue, &common_msg);
	
	while(common_msg.operation_code / 1000 == 5)
	{
		zserver->op->zput_request(zserver, &common_msg);
		//get another cmd message
		queue->op->syn_queue_pop(queue, &common_msg);
	}
	switch(common_msg.operation_code)
	{
		case SERVER_STOP:
			init_server_stop_handler(event_handler, local_server, &common_msg);
			event_handler->handler = server_stop_handler;
			break;
			
		default:
			event_handler->handler = NULL;
	}
	return event_handler->handler;
}

int main(int argc, char *argv[])
{
	int rank;

	log_init("", LOG_DEBUG);
	mpi_init_with_thread(&argc, &argv);
	rank = get_mpi_rank();

	if(rank == 0)
	{
		rpc_server_t *server = create_rpc_server(1, 8, 0,
				resolve_handler);

		zserver = create_zserver(1);
		zserver->op->zserver_start(zserver);
		local_server = server;
		server->op->server_start(server);
		printf("server will be stopped\n");

		destroy_rpc_server(server);
		zserver->op->zserver_stop(zserver);
		destroy_zserver(zserver);
	}
	else
	{
		zclient_t *zclient;
		sds path, data, return_name, return_data;
		int ret_num;
		stop_server_msg_t *stop_server_msg = NULL;

		zclient = create_zclient(rank);
		zclient->op->start_zclient(zclient);

		path = sds_new("/data/research.bat");
		data = sds_new("This is first test");
		return_name = sds_new_len(NULL, MAX_RET_DATA_LEN);
		return_data = sds_new_len(NULL, MAX_RET_DATA_LEN);
		set_zclient(zclient, 0, 13);
		ret_num = zclient->op->create_parent(zclient, path, data, PERSISTENT, return_name);
		if(ret_num != ZOK)
			printf("something wrong happens\n");
		else
			printf("retrun name is %s\n", return_name);

		path = sds_cpy(path, "/data/research.bat:lock");
		data = sds_cpy(data, "This is a lock");
		ret_num = zclient->op->create_parent(zclient, path, data, PERSISTENT, return_name);
		if(ret_num != ZOK)
			printf("something wrong happens\n");
		else
			printf("retrun name is %s\n", return_name);

		path = sds_cpy(path, "/data/research.bat:lock/lock-");
		data = sds_cpy(data, "This is a lock");
		ret_num = zclient->op->create_znode(zclient, path, data,
				EPHEMERAL_SQUENTIAL, return_name);
		if(ret_num != ZOK)
			printf("something wrong happens\n");
		else
			printf("retrun name is %s\n", return_name);

		path = sds_cpy(path, "/data/research.bat");
		ret_num = zclient->op->get_znode(zclient, path, return_data,
				NULL, 0, NULL, NULL);
		if(ret_num != ZOK)
			printf("something wrong happens\n");
		else
			printf("retrun data is %s\n", return_data);

		path = sds_cpy(path, "/data/research.bat");
		ret_num = zclient->op->get_znode(zclient, path, return_data,
				NULL, 1, watch_handler_change, NULL);
		if(ret_num != ZOK)
			printf("something wrong happens\n");
		else
			printf("retrun data is %s\n", return_data);

		path = sds_cpy(path, "/data/research.bat");
		ret_num = zclient->op->exists_znode(zclient, path, NULL,
				2, watch_handler_delete, NULL);
		if(ret_num != ZOK)
			printf("something wrong happens\n");
		else
			printf("exist ok\n");

		path = sds_cpy(path, "/data/research.bat:watch");
		ret_num = zclient->op->get_children(zclient, path, return_data);
		if(ret_num != ZOK)
			printf("something wrong happens\n");
		else
			printf("children is %s\n", return_data);

		path = sds_cpy(path, "/data/research.bat");
		data = sds_cpy(data, "This is first test!!!");
		ret_num = zclient->op->set_znode(zclient, path, data, -1);
		if(ret_num != ZOK)
			printf("something wrong happens\n");
		else
			printf("set ok wait for watch event\n");

		path = sds_cpy(path, "/data/research.bat");
		data = sds_cpy(data, "This is first test!!!!!!!!!");
		ret_num = zclient->op->set_znode(zclient, path, data, -1);
		if(ret_num != ZOK)
			printf("something wrong happens\n");
		else
			printf("set ok\n");

		path = sds_cpy(path, "/data/research.bat");
		ret_num = zclient->op->delete_znode(zclient, path, -1);
		if(ret_num != ZOK)
			printf("something wrong happens\n");
		else
			printf("deleted wait for watch event\n");

		stop_server_msg = zmalloc(MAX_CMD_MSG_LEN);
		stop_server_msg->operation_code = SERVER_STOP;
		stop_server_msg->source = 1;
		stop_server_msg->tag = 13;
		zclient->rpc_client->op->set_send_buff(zclient->rpc_client,
				stop_server_msg);
		zclient->rpc_client->op->execute(zclient->rpc_client, STOP_SERVER);

		sds_free(path);
		sds_free(data);
		sds_free(return_name);
		sds_free(return_data);
		zfree(stop_server_msg);
		zclient->op->stop_zclient(zclient);
		destroy_zclient(zclient);
	}
	mpi_finish();
	return 0;
}
