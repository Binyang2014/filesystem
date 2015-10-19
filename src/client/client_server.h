/*
 *created on: 2015.10.15
 *author: Binyang
 *This file is going to connect between client and the framwork
 */

#ifndef CLIENT_SERVER_H_
#define CLIENT_SERVER_H_

#include "client_struct.h"
#include "rpc_client.h"
#include "shmem.h"

struct fclient;

struct fclient_ops
{
	int (*f_create)();
	int (*f_open)();
	int (*f_read)();
	int (*f_write)();
}

struct fclient
{
	rpc_client_t *rpc_client;
	shmem_t *shmem;
	struct fclient_ops *fclient_ops;
};

typedef struct fclient fclient_t;
typedef struct fclient_ops fclient_ops_t;

fclient_t *create_fclient(int client_id, int target);
void destroy_fclient(fclient_t *fclient);
#endif
