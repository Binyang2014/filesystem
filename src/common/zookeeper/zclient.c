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
#include "basic_list.h"

//========================PRIVATE FUNCTIONS DECLARE========================
//return unique watch number
static int get_watch_num(zclient_t *zclient);
//add handler to watch list
static int add_to_watch_list(zclient_t *zclient, int watch_type, int watch_code,
		watch_handler_t watch_handler)
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
		znode_status_t *status, int watch_flag, watch_handler_t watch_handler);
//get to know if the znode exist and add watcher for this znode
static int exists_znode(zclient *zclient, const sds path, znode_status_t
		*status, int watch_flag, watch_handler_t watch_handler);

//there will be a thread to handle watch event
static void handle_watch_event(zclient_t *zclient);

//=======================IMPLEMENT PRIVATE FUNCTIONS=======================
//we just assume one zoo client will be excuted by only one thread
//otherwise we should add mutex to this function
static int get_watch_num(zclient_t *zclient)
{
	return (zclient->unique_watch_num++) % MAX_WATCH_CODE;
}

static int add_to_watch_list(zclient_t *zclient, int watch_type, int watch_code,
		watch_handler_t watch_handler)
{
	watch_node_t *watch_node;
	list_t *watch_list = zclient->watch_list;

	watch_node = zmalloc(sizeof(watch_node_t));
	watch_node->watch_type = watch_type;
	watch_node->watch_code = watch_code;
	watch_node->watch_handler = watch_handler;
	watch_list->list_ops->list_add_node_tail(watch_list, watch_node);
}

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
		znode_status_t *status, int watch_flag, watch_handler_t watch_handler)
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
	get_msg->watch_code = get_watch_num(zclient);
	get_msg->unique_tag = rpc_client->tag;
	strcpy(set_msg->path, path);

	//recv zsever return and put it into recv buffer
	rpc_client->op->set_send_buff(rpc_client, get_msg);
	rpc_client->op->execute(rpc_client, COMMAND_WITHOUT_RETURN);
	recv_msg(zreturn, client->target, client->tag);

	if(zreturn->return_code & ZOK)
	{
		sds_cpy(return_data, zreturn->data);
		zstatus_dup(status, &zreturn->status);
	}
	if(!(zreturn->return_code & ZSET_WATCH_ERROR))
		add_to_watch_list(zclient, watch_flag, get_msg->watch_code,
				watch_handler);

	return zreturn->return_code;
}

static int exists_znode(zclient *zclient, const sds path, znode_status_t
		*status, int watch_flag, watch_handler_t watch_handler)
{
	rpc_client_t *rpc_client;
	zoo_exists_znode_t *exists_msg;
	zreturn_mid_t *zreturn;

	rpc_client = zclient->rpc_client;
	exists_msg = zclient->send_buff;
	zreturn = (zreturn_mid_t *)client->recv_buff;

	//construct command message and let rpc client to excuse it
	get_msg->operation_code = ZOO_EXISTS_CODE;
	get_msg->watch_flag = watch_flag;
	get_msg->watch_code = get_watch_num(zclient);
	get_msg->unique_tag = rpc_client->tag;
	strcpy(set_msg->path, path);

	//recv zsever return and put it into recv buffer
	rpc_client->op->set_send_buff(rpc_client, exists_msg);
	rpc_client->op->execute(rpc_client, COMMAND_WITHOUT_RETURN);
	recv_msg(zreturn, client->target, client->tag);

	if(zreturn->return_code & ZOK)
		zstatus_dup(status, &zreturn->status);
	if(!(zreturn->return_code & ZSET_WATCH_ERROR))
		add_to_watch_list(zclient, watch_flag, get_msg->watch_code,
				watch_handler);

	return zreturn->return_code;
}

static void handle_watch_event(zclient_t *zclient)
{
	//pop from recv queue
	//deal with the message, find the handler
	//delete this handler from queue and let handler handle this event.
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
	this->unique_watch_num = 1;
	this->watch_list = list_create();
	list_set_free_method(this->watch_list, zfree);
	this->recv_queue = alloc_syn_queue(ZCLIENT_RECV_QUEUE_SIZE,
			sizeof(watch_ret_msg_t));

	return this;
}

void destroy_zclient(zclient_t *zclient)
{
}
