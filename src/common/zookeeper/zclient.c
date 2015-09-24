/* 
 * author: Binyang
 * created on: 2015.9.10
 * This file is going to finish zookeeper client functions whitch defined in
 * zookeeper.h
 */
#include <stdio.h>
#include "zookeeper.h"
#include "zmalloc.h"
#include "sds.h"
#include "log.h"
#include "message.h"

//========================PRIVATE FUNCTIONS DECLARE========================
//just create ordinary znode
static int create_znode(zclient_t *zclient, const sds path, const sds data,
		znode_type_t type, sds return_name);
//create znode which can contain children
static int create_parent(zclient_t *zserver, const sds path, const sds data, znode_type_t
		type, sds return_name);
//delete znode from ztree
static int delete_znode(zclient_t *zserver, const sds path, int version);
//change znode date
static int set_znode(zclient_t *zserver, const sds path, const sds data, int
		version, int watch_type);
//get znode data and can add watcher for this znode
static int get_znode(zclient_t *zserver, const sds path, sds return_data,
		znode_status_t *status);
//=======================IMPLEMENT PRIVATE FUNCTIONS=======================
static int create_znode(zclient_t *zclient, const sds path, const sds data,
		znode_type_t type, sds return_name)
{
	rpc_client_t *rpc_client;

	//construct command message and let rpc client to excuse it
	//use cmd_without_return type to excuse this command

	//recv zsever return and put it into recv buffer
	//return code and get recv buffer
}

static int create_parent(zclient_t *zserver, const sds path, const sds data, znode_type_t
		type, sds return_name);
static int delete_znode(zclient_t *zserver, const sds path, int version);
static int set_znode(zclient_t *zserver, const sds path, const sds data, int
		version, int watch_type)
{
	//in this function you should register watch event to watch list;
}

static int get_znode(zclient_t *zserver, const sds path, sds return_data,
		znode_status_t *status);

static void start()
{
	//in this function you shoud start a thread to excuse watch event if it
	//happened, and start a receive queue to get watch events
}

static void stop()
{
}
//============================PUBLIC INTERFACES============================
init_zclient(zclient_t *zclient, int target, int tag)
{
	zclient->rpc_client = create_rpc_client(zclient-client_id, target, tag);
}

zclient_t *create_zclient(int client_id)
{
	zclient *this = zmalloc(sizeof(zclient_t));

	this->rpc_client = NULL;
	this->client_id = client_id;
	return this;
}

void destroy_zclient(zclient_t *zclient)
{
}
