/*
 * create on: 2015.1019
 * author: Binyang
 * This file will implement functions define in clinet_server.h
 */
#include <string.h>
#include "client_server.h"
#include "zmalloc.h"
#include "message.h"

static int code_transfer(uint32_t op_status);
static void f_create(fclient_t *fclient, createfile_msg_t *createfile_msg);
static int f_open(fclient_t *fclient, openfile_msg_t *openfile_msg);

static int code_transfer(uint32_t op_status)
{
	return FOK;
}

static void f_create(fclient_t *fclient, createfile_msg_t *createfile_msg)
{
	rpc_client_t *rpc_cilent = NULL;
	client_create_file_t *cilent_create_file = NULL;
	shmem_t *shmem = NULL;
	file_ret_msg ret_msg;

	//construct client_create_file message
	rpc_client = fclient->rpc_client;
	client_create_file = zmalloc(sizeof(client_create_file));
	if(createfile_msg->open_mode == CREATE_TEMP)
		client_create_file->operation_code = CREATE_TEMP_FILE_CODE;
	else
		client_create_file->operation_code = CREATE_PERSIST_FILE_CODE;;
	client_create_file->file_mode = createfile_msg->mode;
	strcpy(client_create_file->file_name, createfile_msg->file_path);

	//send create message to data master
	rpc_client->op->set_send_buff(rpc_client, client_create_file,
			sizeof(client_create_file_t));
	if(rpc_client->op->execute(rpc_client, COMMAND_WITH_RETURN) < 0)
		ret_msg.ret_code = FSERVER_ERR;
	shmem = fclient->shmem;
	ret_msg.ret_code = code_transfer(((acc_msg_t
					*)rpc_client->recv_buff)->op_status);
	shmem->send_to_shm(shmem, &ret_msg, sizeof(file_ret_msg));

	zfree(client_create_file);
	zfree(rpc_client->recv_buff);
}

//open a exist file
static int f_open(fclient_t *fclient, openfile_msg_t *openfile_msg)
{
	rpc_client_t *rpc_client = NULL;
	client_open_file_t *client_open_file = NULL;
	shmem_t *shmem = NULL;
	file_ret_msg ret_msg;
}

//create file client server and need to know data-master id and tag
fclient_t *create_fclient(int client_id, int target, int tag)
{
	fclient_t *fclient = zmalloc(sizeof(fclient_t));

	if(fclient == NULL)
		return NULL;
	fclient->rpc_client = create_rpc_client(client_id, target, tag);
	fclient->shmem = create_shm(COMMON_KEY, MAX_SHM_SIZE, SHM_UREAD | SHM_UWRITE);
	if(fclient->shmem == NULL)
	{
		destroy_rpc_client(fclient->rpc_client);
		zfree(fclient);
		return NULL;
	}
	fclient->zclient = create_zclient(client_id);
	set_zclient(fclient->zclient, target, tag);
	fclient->fclient_ops = zmalloc(sizeof(fclient_ops_t));
	fclient->fclient_ops->f_create = f_create;

	return fclient;
}

void destroy_fclient(fclient_t *fclient)
{
	destroy_shm(fclient->shmem);
	destroy_rpc_client(fclient->rpc_client);
	zfree(fclient);
}
