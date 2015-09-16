/*
 * author: Binyang
 * created on: 2015.9.10
 * This file is going to finish zookeeper server functions whitch defined in
 * zookeeper.h
 */
#include <stdio.h>
#include <assert.h>
#include "zookeeper.h"
#include "sds.h"
#include "log.h"
#include "rpc_server.h"
#include "threadpool.h"

//=======================PRIVATE FUNCTIONS DECLARE=====================
static void set_watcher();
static void remove_watcher();

static void put_request();
static int create_znode(zserver_t *zserver, sds path, sds data, znode_type_t
		type, sds return_name);
static int create_parent(zserver_t *zserver, sds path, sds data, znode_type_t
		type, sds return_name);
static int delete_znode(zserver_t *zserver, sds path, int version)
static int set_znode(zserver_t *zserver, sds path, sds data, int version);
static void get_znode();
static void notify_watcher();
static void start();
static void stop();

//=====================IMPLEMENT PRIVATE FUNCTIONS======================
static int create_znode(zserver_t *zserver, sds path, sds data, znode_type_t
		type, sds return_name)
{
	zvalue_t *zvalue;
	ztree_t *ztree;

	zvalue = create_zvalue(sds data, type, 1);
	if(zvalue == NULL)
		return -1;
	ztree = zserver->ztree;
	if(ztree->op->add_znode(tree, path, zvalue, return_name) == -1)
	{
		destroy_zvalue(zvalue);
		return -1;
	}
	destroy_zvalue(zvalue);
	return 0;
}

static int create_parent(zserver_t *zserver, sds path, sds data, znode_type_t
		type, sds return_name)
{
	zvalue_t *zvalue;
	ztree_t *ztree;

	zvalue = create_zvalue_parent(sds_data, type, 1);
	if(zvalue == NULL)
		return -1;
	ztree = zserver->ztree;
	if(ztree->op->add_znode(tree, path, zvalue, return_name) == -1)
	{
		destroy_zvalue(zvalue);
		return -1;
	}
	destroy_zvalue(zvalue);
	return 0;
}

static int delete_znode(zserver_t *zserver, sds path, int version)
{
	ztree_t *ztree;
	zvalue_t *zvalue;

	ztree = zserver->ztree;
	if(version == -1)
	{
		ztree->op->delete_znode(ztree, path);
		return 0;
	}
	zvalue = ztree->op->find_znode(ztree, path);
	if(zvalue == NULL)
		return -1;
	if(zvalue->status.version != version)
	{
		log_write(LOG_WARN, "wrong znode version");
		destroy_zvalue(zvalue);
		return -1;
	}
	destroy_zvalue(zvalue);
	ztree->op->delete_znode(ztree, path);
	return 0;
}

static int set_znode(zserver_t *zserver, sds path, sds data, int version)
{
	ztree_t *ztree;
	zvalue_t *zvalue;
	sds old_data;

	ztree = zserver->ztree;
	zvalue = ztree->op->find_znode(ztree, path);
	if(version == -1)
	{
		old_data = zvalue->data;
		zvalue->data = sds_dup(data);
		sds_free(old_data);
		destroy_zvalue(zvalue);
		return 0;
	}
	if(zvalue->status.version != version)
	{
		log_write(LOG_WARN, "wrong znode version");
		destroy_zvalue(zvalue);
		return -1;
	}
	old_data = zvalue->data;
	zvalue->data = sds_dup(data);
	sds_free(old_data);
	destroy_zvalue(zvalue);
	return 0;
}
//===========================PUBLIC FUNCTIONS===========================
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
