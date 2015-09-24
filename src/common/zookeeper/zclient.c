/* 
 * author: Binyang
 * created on: 2015.9.10
 * This file is going to finish zookeeper client functions whitch defined in
 * zookeeper.h
 */
#include <stdio.h>
#include <string.h>
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
static int create_parent(zclient_t *zclient, const sds path, const sds data, znode_type_t
		type, sds return_name);
//delete znode from ztree
static int delete_znode(zclient_t *zclient, const sds path, int version);
//change znode date
static int set_znode(zclient_t *zclient, const sds path, const sds data, int
		version);
//get znode data and can add watcher for this znode
static int get_znode(zclient_t *zclient, const sds path, sds return_data,
		znode_status_t *status);

//=======================IMPLEMENT PRIVATE FUNCTIONS=======================
static int create_znode(zclient_t *zclient, const sds path, const sds data,
		znode_type_t type, sds return_name)
{
	rpc_client_t *rpc_client;
	zoo_create_znode_t *create_msg;
	zreturn_sim_t *zreturn;

	rpc_client = zclient->rpc_client;
	create_msg = zclient->send_buff;
	zreturn = (zreturn_sim_t *)client->recv_buff;

	//construct command message and let rpc client to excuse it
	create_msg->operation_code = ZOO_CREATE_CODE;
	create_msg->znode_type = type;
	create_msg->unique_tag = rpc_client->tag;
	strcpy(create_msg->path, path);
	strcpy(create_msg->data, data);

	//recv zsever return and put it into recv buffer
	rpc_client->op->set_send_buff(rpc_client, create_msg);
	rpc_client->op->execute(rpc_client, COMMAND_WITHOUT_RETURN);
	recv_msg(zreturn, client->target, client->tag);
	if(zreturn->return_code & ZOK)
		sds_cpy(return_name, zreturn->data);

	return zreturn->return_code;
}

static int create_parent(zclient_t *zclient, const sds path, const sds data, znode_type_t
		type, sds return_name)
{
	rpc_client_t *rpc_client;
	zoo_create_znode_t *create_msg;
	zreturn_sim_t *zreturn;

	rpc_client = zclient->rpc_client;
	create_msg = zclient->send_buff;
	zreturn = (zreturn_sim_t *)client->recv_buff;

	//construct command message and let rpc client to excuse it
	create_msg->operation_code = ZOO_CREATE_PARENT_CODE;
	create_msg->znode_type = type;
	create_msg->unique_tag = rpc_client->tag;
	strcpy(create_msg->path, path);
	strcpy(create_msg->data, data);

	//recv zsever return and put it into recv buffer
	rpc_client->op->set_send_buff(rpc_client, create_msg);
	rpc_client->op->execute(rpc_client, COMMAND_WITHOUT_RETURN);
	recv_msg(zreturn, client->target, client->tag);
	if(zreturn->return_code & ZOK)
		sds_cpy(return_name, zreturn->data);

	return zreturn->return_code;
}

static int delete_znode(zclient_t *zclient, const sds path, int version)
{
	rpc_client_t *rpc_client;
	zoo_delete_znode_t *delete_msg;
	zreturn_base_t *zreturn;

	rpc_client = zclient->rpc_client;
	delete_msg = zclient->send_buff;
	zreturn = (zreturn_base_t *)client->recv_buff;

	//construct command message and let rpc client to excuse it
	delete_msg->operation_code = ZOO_DELETE_CODE;
	delete_msg->version = version;
	delete_msg->unique_tag = rpc_client->tag;
	strcpy(delete_msg->path, path);

	//recv zsever return and put it into recv buffer
	rpc_client->op->set_send_buff(rpc_client, delete_msg);
	rpc_client->op->execute(rpc_client, COMMAND_WITHOUT_RETURN);
	recv_msg(zreturn, client->target, client->tag);

	return zreturn->return_code;
}

static int set_znode(zclient_t *zclient, const sds path, const sds data, int
		version)
{
	rpc_client_t *rpc_client;
	zoo_set_znode_t *set_msg;
	zreturn_base_t *zreturn;

	rpc_client = zclient->rpc_client;
	set_msg = zclient->send_buff;
	zreturn = (zreturn_base_t *)client->recv_buff;

	//construct command message and let rpc client to excuse it
	set_msg->operation_code = ZOO_SET_CODE;
	set_msg->version = version;
	set_msg->unique_tag = rpc_client->tag;
	strcpy(set_msg->path, path);
	strcpy(set_msg->data, data);

	//recv zsever return and put it into recv buffer
	rpc_client->op->set_send_buff(rpc_client, delete_msg);
	rpc_client->op->execute(rpc_client, COMMAND_WITHOUT_RETURN);
	recv_msg(zreturn, client->target, client->tag);

	return zreturn->return_code;
}

static int get_znode(zclient_t *zclient, const sds path, sds return_data,
		znode_status_t *status, int watch_flag)
{
	rpc_client_t *rpc_client;
	zoo_get_znode_t *get_msg;
	zreturn_complex_t *zreturn;

	rpc_client = zclient->rpc_client;
	get_msg = zclient->send_buff;
	zreturn = (zreturn_complex_t *)client->recv_buff;

	//construct command message and let rpc client to excuse it
	get_msg->operation_code = ZOO_GET_CODE;
	get_msg->watch_flag = watch_flag;
	//TODO modify it
	get_msg->watch_code = zclient->unique_watch_num++;
	get_msg->unique_tag = rpc_client->tag;
	strcpy(set_msg->path, path);

	//recv zsever return and put it into recv buffer
	rpc_client->op->set_send_buff(rpc_client, delete_msg);
	rpc_client->op->execute(rpc_client, COMMAND_WITHOUT_RETURN);
	recv_msg(zreturn, client->target, client->tag);

	if(zreturn->return_code & ZOK)
	{
		sds_cpy(return_data, zreturn->data);
		zstatus_dup(status, &zreturn->status);
	}
	if(!(zreturn->return_code & ZSET_WATCH_ERROR))
	{
		//TODO add watch to list
	}

	return zreturn->return_code;
}

static void start()
{
	//in this function you shoud start a thread to excuse watch event if it
	//happened, and start a receive queue to get watch events
}

static void stop()
{
}
//============================PUBLIC INTERFACES============================
void init_zclient(zclient_t *zclient, int target, int tag)
{
	zclient->rpc_client = create_rpc_client(zclient-client_id, target, tag);
}

zclient_t *create_zclient(int client_id)
{
	zclient *this = zmalloc(sizeof(zclient_t));

	this->rpc_client = NULL;
	this->client_id = client_id;
	this->send_buff = zmalloc(sizeof(MAX_CMD_MSG_LEN));

	return this;
}

void destroy_zclient(zclient_t *zclient)
{
}
