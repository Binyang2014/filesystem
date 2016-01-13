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
#include "fifo_ipc.h"
#include "bitmap.h"
#include "sds.h"

#define MAX_FD_NUMBER 1024

struct fclient
{
	rpc_client_t *rpc_client;
	zclient_t *zclient;
	list_t *file_list;
	unsigned long *bitmap;
	int data_master_id;
};

typedef struct fclient fclient_t;

fclient_t *create_fclient(int client_id, int target, int tag);

int fs_create(createfile_msg_t *createfile_msg, int *ret_fd);
int fs_open(openfile_msg_t *openfile_msg, int *ret_fd);
int fs_append( appendfile_msg_t *writefile_msg, const char *data);
int fs_read(readfile_msg_t *readfile_msg, char *data);
int fs_close(closefile_msg_t *closefile_msg);
int fs_remove(removefile_msg_t *removefile_msg);

void destroy_fclient();
void *fclient_run();
#endif
