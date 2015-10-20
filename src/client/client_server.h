/*
 *created on: 2015.10.15
 *author: Binyang
 *This file is going to connect between client and the framwork
 */

#ifndef CLIENT_SERVER_H_
#define CLIENT_SERVER_H_

#include "zookeeper.h"
#include "client_struct.h"
#include "rpc_client.h"
#include "basic_list.h"
#include "shmem.h"
#include "bitmap.h"

#define MAX_FD_NUMBER 1024

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
	zclient_t *zclient;
	struct fclient_ops *fclient_ops;
	list_t *file_list;
	unsigned long *bitmap;
	int data_master_id;
};

typedef struct fclient fclient_t;
typedef struct fclient_ops fclient_ops_t;

fclient_t *create_fclient(int client_id, int target);
void destroy_fclient(fclient_t *fclient);
#endif
