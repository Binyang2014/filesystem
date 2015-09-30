/* 
 * author: Binyang
 * created on: 2015.9.10
 * This file is going to finish zookeeper client functions whitch defined in
 * zookeeper.h
 */
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include "zookeeper.h"
#include "zmalloc.h"
#include "sds.h"
#include "log.h"
#include "message.h"
#include "basic_list.h"

static int zclient_stop_commit = 0;
//========================PRIVATE FUNCTIONS DECLARE========================
//return unique watch number
static int get_watch_num(zclient_t *zclient);
//watch list match
static int watch_node_match(void *ptr, void *key);
//watch list free
static void watch_node_free(void* watch_node);

//add handler to watch list
static void add_to_watch_list(zclient_t *zclient, int watch_type, int watch_code,
		watch_handler_t watch_handler, void *args);
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
		znode_status_t *status, int watch_flag, watch_handler_t watch_handler,
		void* args);
//get to know if the znode exist and add watcher for this znode
static int exists_znode(zclient_t *zclient, const sds path, znode_status_t
		*status, int watch_flag, watch_handler_t watch_handler, void *args);
//get all children in this path
static int get_children(zclient_t *zclient, const sds path, sds return_data);

//there will be a thread to handle watch event
static void *handle_watch_event(void *args);
static void *get_watch_event(void *args);

static void start_zclient(zclient_t *zclient);
static void stop_zclient(zclient_t *zclient);
//=======================IMPLEMENT PRIVATE FUNCTIONS=======================
//we just assume one zoo client will be excuted by only one thread
//otherwise we should add mutex to this function
static int get_watch_num(zclient_t *zclient)
{
	return (zclient->unique_watch_num++) % MAX_WATCH_CODE;
}

static int watch_node_match(void *ptr, void *key)
{
	watch_node_t *node = ptr;
	watch_node_t *watch_key = key;

	if(node->watch_code == watch_key->watch_code)
		return 1;
	return 0;
}

static void watch_node_free(void* watch_node)
{
	watch_node_t *node = watch_node;

	zfree(node->args);
	zfree(node);
}

//we just zfree args after excuting watch handler so do not use malloc in args
static void add_to_watch_list(zclient_t *zclient, int watch_type, int watch_code,
		watch_handler_t watch_handler, void* args)
{
	watch_node_t *watch_node;
	list_t *watch_list = zclient->watch_list;

	watch_node = zmalloc(sizeof(watch_node_t));
	watch_node->watch_type = watch_type;
	watch_node->watch_code = watch_code;
	watch_node->watch_handler = watch_handler;
	watch_node->args = args;
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
	zreturn = (zreturn_sim_t *)zclient->recv_buff;

	//construct command message and let rpc client to excuse it
	create_msg->operation_code = ZOO_CREATE_CODE;
	create_msg->znode_type = type;
	create_msg->unique_tag = rpc_client->tag;
	strcpy((char *)create_msg->path, path);
	strcpy((char *)create_msg->data, data);

	//recv zsever return and put it into recv buffer
	rpc_client->op->set_send_buff(rpc_client, create_msg);
	rpc_client->op->execute(rpc_client, COMMAND_WITHOUT_RETURN);
	recv_msg(zreturn, rpc_client->target, rpc_client->tag, sizeof(zreturn_sim_t));
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
	create_msg = (zoo_create_znode_t *)zclient->send_buff;
	zreturn = (zreturn_sim_t *)zclient->recv_buff;

	//construct command message and let rpc client to excuse it
	create_msg->operation_code = ZOO_CREATE_PARENT_CODE;
	create_msg->znode_type = type;
	create_msg->unique_tag = rpc_client->tag;
	strcpy(create_msg->path, path);
	strcpy(create_msg->data, data);

	//recv zsever return and put it into recv buffer
	rpc_client->op->set_send_buff(rpc_client, create_msg);
	rpc_client->op->execute(rpc_client, COMMAND_WITHOUT_RETURN);
	recv_msg(zreturn, rpc_client->target, rpc_client->tag, sizeof(zreturn_sim_t));
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
	zreturn = (zreturn_base_t *)zclient->recv_buff;

	//construct command message and let rpc client to excuse it
	delete_msg->operation_code = ZOO_DELETE_CODE;
	delete_msg->version = version;
	delete_msg->unique_tag = rpc_client->tag;
	strcpy(delete_msg->path, path);

	//recv zsever return and put it into recv buffer
	rpc_client->op->set_send_buff(rpc_client, delete_msg);
	rpc_client->op->execute(rpc_client, COMMAND_WITHOUT_RETURN);
	recv_msg(zreturn, rpc_client->target, rpc_client->tag, sizeof(zreturn_base_t));

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
	zreturn = (zreturn_base_t *)zclient->recv_buff;

	//construct command message and let rpc client to excuse it
	set_msg->operation_code = ZOO_SET_CODE;
	set_msg->version = version;
	set_msg->unique_tag = rpc_client->tag;
	strcpy(set_msg->path, path);
	strcpy(set_msg->data, data);

	//recv zsever return and put it into recv buffer
	rpc_client->op->set_send_buff(rpc_client, set_msg);
	rpc_client->op->execute(rpc_client, COMMAND_WITHOUT_RETURN);
	recv_msg(zreturn, rpc_client->target, rpc_client->tag, sizeof(zreturn_base_t));

	return zreturn->return_code;
}

static int get_znode(zclient_t *zclient, const sds path, sds return_data,
		znode_status_t *status, int watch_flag, watch_handler_t watch_handler,
		void*args)
{
	rpc_client_t *rpc_client;
	zoo_get_znode_t *get_msg;
	zreturn_complex_t *zreturn;

	rpc_client = zclient->rpc_client;
	get_msg = zclient->send_buff;
	zreturn = (zreturn_complex_t *)zclient->recv_buff;

	//construct command message and let rpc client to excuse it
	get_msg->operation_code = ZOO_GET_CODE;
	get_msg->watch_flag = watch_flag;
	get_msg->watch_code = get_watch_num(zclient);
	get_msg->unique_tag = rpc_client->tag;
	strcpy(get_msg->path, path);

	//recv zsever return and put it into recv buffer
	rpc_client->op->set_send_buff(rpc_client, get_msg);
	rpc_client->op->execute(rpc_client, COMMAND_WITHOUT_RETURN);
	recv_msg(zreturn, rpc_client->target, rpc_client->tag,
			sizeof(zreturn_complex_t));

	if(zreturn->return_code & ZOK)
	{
		sds_cpy(return_data, zreturn->data);
		if(status != NULL)
			zstatus_dup(status, &zreturn->status);
	}
	if(watch_flag && !(zreturn->return_code & ZSET_WATCH_ERROR))
		add_to_watch_list(zclient, watch_flag, get_msg->watch_code,
				watch_handler, args);

	return zreturn->return_code;
}

static int get_children(zclient_t *zclient, const sds path, sds return_data)
{
	rpc_client_t *rpc_client;
	zoo_get_children_t *get_children_msg;
	zreturn_sim_t *zreturn;

	rpc_client = zclient->rpc_client;
	get_children_msg = zclient->send_buff;
	zreturn = (zreturn_sim_t *)zclient->recv_buff;

	//construct command message and let rpc client to excuse it
	get_children_msg->operation_code = ZOO_GET_CHILDREN_CODE;
	get_children_msg->unique_tag = rpc_client->tag;
	strcpy(get_children_msg->path, path);

	//recv zsever return and put it into recv buffer
	rpc_client->op->set_send_buff(rpc_client, get_children_msg);
	rpc_client->op->execute(rpc_client, COMMAND_WITHOUT_RETURN);
	recv_msg(zreturn, rpc_client->target, rpc_client->tag,
			sizeof(zreturn_sim_t));

	if(zreturn->return_code & ZOK)
		sds_cpy(return_data, zreturn->data);

	return zreturn->return_code;
}

static int exists_znode(zclient_t *zclient, const sds path, znode_status_t
		*status, int watch_flag, watch_handler_t watch_handler, void *args)
{
	rpc_client_t *rpc_client;
	zoo_exists_znode_t *exists_msg;
	zreturn_mid_t *zreturn;

	rpc_client = zclient->rpc_client;
	exists_msg = zclient->send_buff;
	zreturn = (zreturn_mid_t *)zclient->recv_buff;

	//construct command message and let rpc client to excuse it
	exists_msg->operation_code = ZOO_EXISTS_CODE;
	exists_msg->watch_flag = watch_flag;
	exists_msg->watch_code = get_watch_num(zclient);
	exists_msg->unique_tag = rpc_client->tag;
	strcpy(exists_msg->path, path);

	//recv zsever return and put it into recv buffer
	rpc_client->op->set_send_buff(rpc_client, exists_msg);
	rpc_client->op->execute(rpc_client, COMMAND_WITHOUT_RETURN);
	recv_msg(zreturn, rpc_client->target, rpc_client->tag, sizeof(zreturn_mid_t));

	if(status != NULL && (zreturn->return_code & ZOK))
		zstatus_dup(status, &zreturn->status);
	if(watch_flag && !(zreturn->return_code & ZSET_WATCH_ERROR))
		add_to_watch_list(zclient, watch_flag, exists_msg->watch_code,
				watch_handler, args);

	return zreturn->return_code;
}

static void clean_up(watch_ret_msg_t *watch_ret_msg)
{
	zfree(watch_ret_msg);
}

static void *handle_watch_event(void *args)
{
	zclient_t *zclient = args;
	watch_ret_msg_t *watch_ret_msg;
	syn_queue_t *recv_queue = zclient->recv_queue;
	list_t *watch_list = zclient->watch_list;
	watch_node_t watch_key, *watch_node;
	list_node_t *node;

	watch_ret_msg = zmalloc(sizeof(watch_ret_msg_t));
	pthread_cleanup_push(clean_up, watch_ret_msg);
	while(1)
	{
		recv_queue->op->syn_queue_pop(recv_queue, watch_ret_msg);
		watch_key.watch_code = atoi(watch_ret_msg->ret_data);
		//This is a stop zclient message
		if(watch_key.watch_code == -1)
			continue;
		node = watch_list->list_ops->list_search_key(watch_list, &watch_key);
		if(node == NULL)
			continue;
		watch_node = node->value;
		watch_node->watch_handler(watch_node->args);
		//delete node from list
		watch_list->list_ops->list_del_node(watch_list, node);
	}
	pthread_cleanup_pop(0);
	return NULL;
}

static void *get_watch_event(void *args)
{
	watch_ret_msg_t *watch_ret_msg;
	zclient_t *zclient = args;

	watch_ret_msg = zmalloc(sizeof(watch_ret_msg_t));
	while(!zclient->client_stop)
	{
		recv_msg(watch_ret_msg, ANY_SOURCE, WATCH_TAG, sizeof(watch_ret_msg_t));
		zclient->recv_queue->op->syn_queue_push(zclient->recv_queue,
				watch_ret_msg);
	}
	zfree(watch_ret_msg);
	zclient_stop_commit = 1;
	return NULL;
}

static void start_zclient(zclient_t *zclient)
{
	pthread_create(&zclient->watch_tid, NULL, handle_watch_event, zclient);
	pthread_create(&zclient->recv_watch_tid, NULL, get_watch_event, zclient);
}

static void stop_zclient(zclient_t *zclient)
{
	watch_ret_msg_t *watch_ret_msg;

	//this request maybe interupt may other request, so make sure you will no
	//get watch event for a while or server has been stopped
	watch_ret_msg = zmalloc(sizeof(watch_ret_msg_t));
	zclient->client_stop = 1;
	strcpy(watch_ret_msg->ret_data, "-1");
	send_msg(watch_ret_msg, zclient->client_id, WATCH_TAG,
			sizeof(watch_ret_msg_t));
	while(!zclient_stop_commit);
	pthread_join(zclient->recv_watch_tid, NULL);

	while(!zclient->recv_queue->queue->basic_queue_op->is_empty(zclient->recv_queue->queue))
		usleep(50);
	pthread_cancel(zclient->watch_tid);
	pthread_join(zclient->watch_tid, NULL);
	pthread_mutex_unlock(zclient->recv_queue->queue_mutex);
	log_write(LOG_INFO, "zclient has been stopped");

	zfree(watch_ret_msg);
}
//============================PUBLIC INTERFACES============================
void set_zclient(zclient_t *zclient, int target, int tag)
{
	zclient->rpc_client->target = target;
	zclient->rpc_client->tag = tag;
}

zclient_t *create_zclient(int client_id)
{
	zclient_t *this = zmalloc(sizeof(zclient_t));
	assert(this != NULL);

	this->rpc_client = NULL;
	this->client_id = client_id;
	this->client_stop = 0;
	this->send_buff = zmalloc(MAX_CMD_MSG_LEN);
	this->recv_buff = zmalloc(sizeof(zreturn_complex_t));
	this->unique_watch_num = 1;
	this->watch_list = list_create();
	list_set_free_method(this->watch_list, watch_node_free);
	list_set_match_method(this->watch_list, watch_node_match);
	this->recv_queue = alloc_syn_queue(ZCLIENT_RECV_QUEUE_SIZE,
			sizeof(watch_ret_msg_t));
	this->rpc_client = create_rpc_client(client_id, -1, -1);

	this->op = zmalloc(sizeof(zclient_op_t));
	this->op->create_znode = create_znode;
	this->op->create_parent = create_parent;
	this->op->delete_znode = delete_znode;
	this->op->set_znode = set_znode;
	this->op->get_znode = get_znode;
	this->op->exists_znode = exists_znode;
	this->op->get_children = get_children;
	this->op->start_zclient = start_zclient;
	this->op->stop_zclient = stop_zclient;

	return this;
}

void destroy_zclient(zclient_t *zclient)
{
	zfree(zclient->op);
	destroy_syn_queue(zclient->recv_queue);
	list_release(zclient->watch_list);
	zfree(zclient->recv_buff);
	zfree(zclient->send_buff);
	if(zclient->rpc_client != NULL)
		destroy_rpc_client(zclient->rpc_client);
	zfree(zclient);
}
