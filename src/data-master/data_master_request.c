/*
 * data_master_request.c
 *
 *  Created on: 2015年11月12日
 *      Author: ron
 */
#include "data_master_request.h"

/*--------------------Private Declaration------------------*/
static void create_tmp_file(data_master_request_t *request, client_create_file_t *c_cmd);
static void *append_temp_file(data_master_request_t *request, client_append_file_t *c_cmd);
static void *read_temp_file(data_master_request_t *request, client_read_file_t *c_cmd);

/*--------------------Private Implementation---------------*/

static void create_tmp_file(data_master_request_t *request, client_create_file_t *c_cmd)
{
	request->client->op->set_send_buff(request->client, c_cmd, sizeof(*c_cmd));
	request->client->op->execute(request->client, COMMAND);
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

data_master_request_t *create_data_master_request(int client_id, int target, int tag)
{
	data_master_request_t *request = zmalloc(sizeof(*request));
	request->client = create_rpc_client(client_id, target, tag);
	request->op = zmalloc(sizeof(data_master_request_op_t));
	request->op->append_temp_file = append_temp_file;
	request->op->create_tmp_file = create_tmp_file;
	request->op->read_temp_file = read_temp_file;

	return request;
}

void destroy_data_master_request(data_master_request_t *request)
{
	destroy_rpc_client(request->client);
	zfree(request->op);
	zfree(request);
	request = NULL;
}


