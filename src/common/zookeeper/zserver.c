/*
 * author: Binyang
 * created on: 2015.9.10
 * This file is going to finish zookeeper server functions whitch defined in
 * zookeeper.h
 */
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "zookeeper.h"
#include "sds.h"
#include "log.h"
#include "rpc_server.h"
#include "threadpool.h"
#include "message.h"

//=======================PRIVATE FUNCTIONS DECLARE=====================
static void remove_watcher();

static void put_request();

//create znode on ztree
static int create_znode(zserver_t *zserver, const sds path, const sds data, znode_type_t
		type, sds return_name);
//create znode which can contain children
static int create_parent(zserver_t *zserver, const sds path, const sds data, znode_type_t
		type, sds return_name);
//delete znode from ztree
static int delete_znode(zserver_t *zserver, const sds path, int version)
//change znode date
static int set_znode(zserver_t *zserver, const sds path, const sds data, int version);
//get znode data and can add watcher for this znode
static int get_znode(zserver_t *zserver, const sds path, sds return_data,
		znode_status_t *status, sds watch_data)
//get to know if the znode exist and can add watcher for this znode
static int exists_znode(zserver_t *zserver, const sds path, znode_status_t
		*status, sds watch_data);
//add watcher to this znode
static int add_watcher(zserver_t *zserver, const sds path, sds watch_data);
//changed happened, so notice associate clients
static int notify_watcher(zserver_t *zserver, const sds path, int notice_type);
//notice child
static int notice_child(zserver_t *zserver, zvalue_t *value, int notice_type);

//resolve message function
void *resolve_handler(event_handler_t *event_handler, void *msg_queue);
static void start();
static void stop();

//=====================IMPLEMENT PRIVATE FUNCTIONS======================

//=======================ABOUT WATCHER FUNCTIONS========================
static int notice_child(zserver_t *zserver, zvalue_t *value, int notice_type)
{
	const sds data;
	sds *information;
	int count;
	watch_data_t watch_data;
	rpc_server_t *rpc_server;
	watch_ret_msg_t *ret_msg;

	data = value->data;
	rpc_server = zserver->rpc_server;
	information = sds_split_len(data, sds_len(data), " ", 1, &count);

	//get watch data
	watch_data->dst = atoi(information[0]);
	watch_data->tag = atoi(information[1]);
	watch_data->watch_type = atoi(information[2]);
	if(watch_data->watch_type && notice_type == 0)
		return ZWRONG_WATCH_TYPE;
	watch_data->unique_code = sds_dup(information[3]);
	sds_free_split_res(information, count);

	//sent watch message to client
	ret_msg = zmalloc(sizeof(watch_ret_msg_t));
	ret_msg->watch_type = watch_data->watch_type;
	if(str_len(watch_data->unique_code) > MAX_RET_DATA_LEN)
	{
		sds_free(watch_data->unique_code);
		zfree(ret_msg);
		return ZRET_MSG_TOO_LONG;
	}
	strcpy(ret_msg->ret_data, watch_data->unique_code);
	rpc_server->op->send_to_queue(rpc_server, ret_msg, watch_data->dst,
			watch_data->tag, sizeof(watch_ret_msg_t));
	sds_free(watch_data->unique_code);
	zfree(ret_msg);
	return ZOK;
}

static int add_watcher(zserver_t *zserver, const sds path, sds watch_data)
{
	sds watch_path;
	zvalue_t *value;
	ztree_t *ztree;

	watch_path = sds_dup(path);
	watch_path = sds_cat(watch_path, "/watch");
	ztree = zserver->ztree;
	value = ztree->op->find_znode(ztree, watch_path);
	if(value == NULL)
	{
		if(create_parent(zserver, watch_path, NULL, PERSISTENT, NULL) != ZOK)
		{
			sds_free(watch_path);
			return ZSET_WATCH_ERROR;
		}
	}
	watch_path = sds_cat(watch, "/watcher-");
	if(create_znode(zserver, watch_path, watch_data, EPHEMERAL_SQUENTIAL, NULL)
			!= ZOK)
	{
		sds_free(watch_path);
		destroy_zvalue(value);
		return ZSET_WATCH_ERROR;
	}
	sds_free(watch_path);
	destroy_zvalue(value);
	return ZOK;
}

static int notify_watcher(zserver_t *zserver, const sds path, int notice_type)
{
	sds watch_path, watcher, *watch_child;
	zvalue_t *value;
	ztree_t ztree;
	int count, i;

	watch_path = sds_dup(path);
	watch_path = sds_cat(watch_path. "/watch");
	ztree = zserver->ztree;
	value = ztree->op->find_znode(ztree, watch_path);
	if(value == NULL)
	{
		sds_free(watch_path);
		return ZOK;
	}
	watch_child = ztree->op->get_children(ztree, watch_path, &count);
	destroy_zvalue(value);
	watcher = sds_dup(watch_path);
	for(i = 0; i < count; i++)
	{
		watcher = sds_cpy(watcher, watch_path);
		watcher = sds_cat(watch_path, "/");
		watcher = sds_cat(watch_path, watch_child[i]);
		value = ztree->op->find_znode(ztree, watcher);
		//notice this child
		if(notice_child(zserver, value, notice_type) != ZOK)
		{
			destroy_zvalue(value);
			continue;
		}
		destroy_zvalue(value);
		ztree->op->delete_znode(ztree, watcher);
	}

	sds_free_split_res(watch_child, count);
	sds_free(watcher);
	destroy_zvalue(value);
	sds_free(watch_path);
	return ZOK;
}

//===================ABOUT OPERATING ZNODE FUNCTIONS=====================
static int create_znode(zserver_t *zserver, const sds path, const sds data, znode_type_t
		type, sds return_name)
{
	zvalue_t *zvalue;
	ztree_t *ztree;

	zvalue = create_zvalue(data, type, 1);
	if(zvalue == NULL)
		return ZCREATE_WRONG;
	ztree = zserver->ztree;
	if(ztree->op->add_znode(tree, path, zvalue, return_name) == -1)
	{
		destroy_zvalue(zvalue);
		return ZCREATE_WRONG;
	}
	destroy_zvalue(zvalue);
	return ZOK;
}

static int create_parent(zserver_t *zserver, const sds path, const sds data, znode_type_t
		type, sds return_name)
{
	zvalue_t *zvalue;
	ztree_t *ztree;

	zvalue = create_zvalue_parent(data, type, 1);
	if(zvalue == NULL)
		return ZCREATE_WRONG;
	ztree = zserver->ztree;
	if(ztree->op->add_znode(tree, path, zvalue, return_name) == -1)
	{
		destroy_zvalue(zvalue);
		return ZCREATE_WRONG;
	}
	destroy_zvalue(zvalue);
	return ZOK;
}

static int delete_znode(zserver_t *zserver, const sds path, int version)
{
	ztree_t *ztree;
	zvalue_t *zvalue;

	ztree = zserver->ztree;
	zvalue = ztree->op->find_znode(ztree, path);
	if(zvalue == NULL)
		return ZNO_EXISTS;
	if(version == -1)
	{
		destroy_zvalue(zvalue);
		notify_watcher(zserver, path, NOTICE_DELETE);
		ztree->op->delete_znode(ztree, path);
		return ZOK;
	}
	if(zvalue->status.version != version)
	{
		destroy_zvalue(zvalue);
		return ZWRONG_VERSION;
	}
	destroy_zvalue(zvalue);
	ztree->op->delete_znode(ztree, path);
	return ZOK;
}

static int set_znode(zserver_t *zserver, const sds path, const sds data, int version)
{
	ztree_t *ztree;
	zvalue_t *zvalue;
	sds old_data;

	ztree = zserver->ztree;
	zvalue = ztree->op->find_znode(ztree, path);
	if(zvalue == NULL)
		return ZNO_EXISTS;
	if(version == -1)
	{
		old_data = zvalue->data;
		zvalue->data = sds_dup(data);
		zvalue->update_znode_status(&zvalue->status, zvalue->status.version + 1,
				ZNODE_MODIFY);
		sds_free(old_data);
		notify_atcher(zserver, path, NOTICE_CHANGED);
		destroy_zvalue(zvalue);
		return ZOK;
	}
	if(zvalue->status.version != version)
	{
		destroy_zvalue(zvalue);
		return ZWRONG_VERSION;
	}
	old_data = zvalue->data;
	zvalue->data = sds_dup(data);
	zvalue->update_znode_status(&zvalue->status, zvalue->status.version + 1,
			ZNODE_MODIFY);
	sds_free(old_data);
	destroy_zvalue(zvalue);
	return ZOK;
}

static int exists_znode(zserver_t *zserver, const sds path, znode_status_t
		*status, sds watch_data)
{
	zvalue_t *value;
	ztree_t *ztree;

	ztree = zserver->ztree;
	value = ztree->op->find_znode(ztree, path);
	if(value == NULL)
		return ZNO_EXISTS;

	//get znode status
	value->update_znode_status(&value->status, ZNODE_ACCESS);
	zstatus_dup(status, &value->status);
	destroy_zvalue(value);

	//set watcher
	if(watch_data == NULL)
		return ZOK;
	if(add_watcher(zserver, path, watch_data) != ZOK)
		return ZOK | ZSET_WATCH_ERROR;
	return ZOK;
}

//You should use sds_new_len to got return data
static int get_znode(zserver_t *zserver, const sds path, int watch, sds return_data,
		znode_status_t *status, sds watch_data)
{
	zvalue_t *value;
	ztree_t *ztree;

	ztree = zserver->ztree;
	value = ztree->op->find_znode(ztree, path);
	if(value == NULL)
		return ZNO_EXISTS;

	//get znode data
	value->update_znode_status(&value->status, ZNODE_ACCESS);
	zstatus_dup(status, &value->status);
	if(sds_len(return_data) < sds_len(value->data))
	{
		destroy_zvalue(value);
		return ZNO_ENOUGH_BUFFER;
	}
	return_data = sds_cpy(return_data, value->data);
	destroy_zvalue(value);

	//set watcher
	if(watch_data == NULL)
		return ZOK;
	if(add_watcher(zserver, path, watch_data) != ZOK)
		return ZOK | ZSET_WATCH_ERROR;
	return ZOK;
}

//===========================PUBLIC FUNCTIONS===========================
void *resolve_handler(event_handler_t *event_handler, void *msg_queue)
{
	common_msg_t common_msg;
}

zserver_t *create_zserver(int server_id, resolve_handler_t resolve_handler)
{
	zserver_t *zserver;

	zserver = zmalloc(sizeof(zserver_t));
	assert(zserver != NULL);
	zserver->op = zmalloc(sizeof(zserver_op_t));
	zserver->ztree = create_ztree(1);
	zserver->rpc_server = create_rpc_server2(1, RECV_QUEUE_SIZE,
			SEND_QUEUE_SIZE, server_id, resolve_handler);
}

zserver_t *destroy_zserver(zserver_t *zserver)
{
	destroy_rpc_server(zserver->rpc_server);
	destroy_ztree(zserver->ztree);
	zfree(zserver->op);
	zfree(zserver);
}
