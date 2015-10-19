/*
 * create on: 2015.1019
 * author: Binyang
 * This file will implement functions define in clinet_server.h
 */
#include "client_server.h"
#include "zmalloc.h"
#include "message.h"

static int f_create();
static int f_open();

static int f_create(fclient_t *fclient, createfile_msg_t *createfile_msg)
{
	rpc_client_t *rpc_cilent;

	rpc_client = fclient->rpc_client;
	return 0;
}

fclient_t *create_fclient(int client_id, int target)
{
	fclient_t *fclient = zmalloc(sizeof(fclient_t));

	if(fclient == NULL)
		return NULL;
	fclient->rpc_client = create_rpc_client(client_id, target, -1);
	fclient->shmem = create_shm(COMMON_KEY, MAX_SHM_SIZE, SHM_UREAD | SHM_UWRITE);
	if(fclient->shmem == NULL)
	{
		destroy_rpc_client(fclient->rpc_client);
		zfree(fclient);
		return NULL;
	}
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
