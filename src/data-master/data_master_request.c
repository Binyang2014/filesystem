/*
 * data_master_request.c
 *
 *  Created on: 2015年11月12日
 *      Author: ron
 */
#include <stdio.h>
#include "data_master_request.h"
#include "zmalloc.h"

/*--------------------Private Declaration------------------*/
static void create_tmp_file(data_master_request_t *request, client_create_file_t *c_cmd);
static void *append_temp_file(data_master_request_t *request, client_append_file_t *c_cmd);
static void *read_temp_file(data_master_request_t *request, client_read_file_t *c_cmd);
static void register_to_master(data_master_request_t *request, c_d_register_t *c_cmd);

/*--------------------Private Implementation---------------*/

static void create_tmp_file(data_master_request_t *request, client_create_file_t *c_cmd)
{
	request->client->op->set_send_buff(request->client, c_cmd, sizeof(*c_cmd));
	request->client->op->execute(request->client, COMMAND_WITHOUT_RETURN);
}

static void *append_temp_file(data_master_request_t *request, client_append_file_t *c_cmd)
{
	request->client->op->set_send_buff(request->client, c_cmd, sizeof(*c_cmd));
	request->client->op->execute(request->client, COMMAND_WITH_RETURN);
	return request->client->recv_buff;
}

static void *read_temp_file(data_master_request_t *request, client_read_file_t *c_cmd)
{
	request->client->op->set_send_buff(request->client, c_cmd, sizeof(*c_cmd));
	request->client->op->execute(request->client, COMMAND_WITH_RETURN);
	return request->client->recv_buff;
}

static void register_to_master(data_master_request_t *request, c_d_register_t *c_cmd)
{
	request->client->op->set_send_buff(request->client, c_cmd, sizeof(*c_cmd));
	request->client->op->execute(request->client, COMMAND_WITHOUT_RETURN);
}

static void stop_master(struct data_master_request *request)
{
	stop_server_msg_t* stop_server_msg = zmalloc(sizeof(stop_server_msg_t));
	stop_server_msg->operation_code = SERVER_STOP;
	stop_server_msg->source = request->client->client_id;
	stop_server_msg->tag = request->client->tag;
	request->client->op->set_send_buff(request->client, stop_server_msg, sizeof(stop_server_msg_t));
	if(request->client->op->execute(request->client, STOP_SERVER) < 0)
	{
		printf("something wrong\n");
	}
	else
	{
		printf("Have stop the server!!!\n");
	}
}

data_master_request_t *create_data_master_request(int client_id, int target, int tag)
{
	data_master_request_t *request = zmalloc(sizeof(*request));
	request->client = create_rpc_client(client_id, target, tag);
	request->op = zmalloc(sizeof(data_master_request_op_t));
	request->op->append_temp_file = append_temp_file;
	request->op->create_tmp_file = create_tmp_file;
	request->op->read_temp_file = read_temp_file;
	request->op->register_to_master = register_to_master;
	request->op->stop_master = stop_master;

	return request;
}

void destroy_data_master_request(data_master_request_t *request)
{
	puts("destroy_data_master_request");
	destroy_rpc_client(request->client);
	puts("destroy_data_master_request");
	zfree(request->op);
	zfree(request);
	request = NULL;
}
