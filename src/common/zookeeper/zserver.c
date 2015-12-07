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
#include "zmalloc.h"

static zserver_t *local_zserver;
//=======================PRIVATE FUNCTIONS DECLARE=====================
//create znode on ztree
static int create_znode(zserver_t *zserver, const sds path, const sds data, znode_type_t
		type, sds return_name);
//create znode which can contain children
static int create_parent(zserver_t *zserver, const sds path, const sds data, znode_type_t
		type, sds return_name);
//delete znode from ztree
static int delete_znode(zserver_t *zserver, const sds path, int version);
//change znode date
static int set_znode(zserver_t *zserver, const sds path, const sds data, int version);
//get znode data and can add watcher for this znode
static int get_znode(zserver_t *zserver, const sds path, sds return_data,
		znode_status_t *status, sds watch_data);
//get znode children
static int get_children(zserver_t *zserver, sds path, sds return_data);
//get to know if the znode exist and can add watcher for this znode
static int exists_znode(zserver_t *zserver, const sds path, znode_status_t
		*status, sds watch_data);
//add watcher to this znode
static int add_watcher(zserver_t *zserver, const sds path, sds watch_data);
//changed happened, so notice associate clients
static int notify_watcher(zserver_t *zserver, const sds path, int notice_type);
//notice child
static int notice_child(zserver_t *zserver, zvalue_t *value, int notice_type,
		zvalue_t *watch_node);

//resolve message function
static void *resolve_handler(event_handler_t *event_handler, void *msg_queue);
static int init_znode_handler(event_handler_t *event_handler, zserver_t
		*zserver, void* common_msg);
//message handler
static void create_znode_handler(event_handler_t *event_handler);
static void create_parent_handler(event_handler_t *event_handler);
static void delete_znode_handler(event_handler_t *event_handler);
static void set_znode_handler(event_handler_t *event_handler);
static void get_znode_handler(event_handler_t *event_handler);
static void exists_znode_handler(event_handler_t *event_handler);
static void get_children_handler(event_handler_t *event_handler);

static void zput_request(zserver_t *zserver, common_msg_t *common_msg);
static void zserver_start(zserver_t *zserver);
static void zserver_stop(zserver_t *zserver);

//=====================IMPLEMENT PRIVATE FUNCTIONS======================

//=======================ABOUT WATCHER FUNCTIONS========================
static void print_watch(watch_ret_msg_t* ret_msg)
{
	log_write(LOG_DEBUG, "watch type is %d, return data is %s",
			ret_msg->watch_type, ret_msg->ret_data);
	log_write(LOG_DEBUG, "The status is: version: %d, create time: %llu, modified time: %llu, access time: %llu",
			ret_msg->status.version, ret_msg->status.created_time,
			ret_msg->status.modified_time, ret_msg->status.access_time);
	
}

static int notice_child(zserver_t *zserver, zvalue_t *value, int notice_type, zvalue_t *watch_node)
{
	sds data;
	sds *information;
	int count;
	watch_data_t watch_data;
	rpc_server_t *rpc_server;
	watch_ret_msg_t *ret_msg;

	data = value->data;
	rpc_server = zserver->rpc_server;
	information = sds_split_len(data, sds_len(data), " ", 1, &count);

	//get watch data
	watch_data.dst = atoi(information[0]);
	watch_data.tag = atoi(information[1]);
	watch_data.watch_type = atoi(information[2]);
	if((watch_data.watch_type & notice_type) == 0)
	{
		sds_free_split_res(information, count);
		return ZWRONG_WATCH_TYPE;
	}
	watch_data.unique_code = sds_dup(information[3]);
	sds_free_split_res(information, count);

	//sent watch message to client
	ret_msg = zmalloc(sizeof(watch_ret_msg_t));
	ret_msg->watch_type = watch_data.watch_type;
	if(sds_len(watch_data.unique_code) > MAX_RET_DATA_LEN)
	{
		sds_free(watch_data.unique_code);
		zfree(ret_msg);
		return ZRET_MSG_TOO_LONG;
	}
	strcpy(ret_msg->ret_data, watch_data.unique_code);
	zstatus_dup(&ret_msg->status, &watch_node->status);
	//for debug
#if ZSERVER_DEBUG
	print_watch(ret_msg);
#endif

	rpc_server->op->send_to_queue(rpc_server, ret_msg, watch_data.dst, watch_data.tag, sizeof(watch_ret_msg_t));
	sds_free(watch_data.unique_code);
	zfree(ret_msg);
	return ZOK;
}

static int add_watcher(zserver_t *zserver, const sds path, sds watch_data)
{
	sds watch_path;
	zvalue_t *value;
	ztree_t *ztree;
	int i;

	watch_path = sds_dup(path);
	//maybe source is like /tmp/test1
	for(i = sds_len(watch_path) - 1; i >= 0 && watch_path[i] != ':'; i--);
	if(i < 0)
		watch_path = sds_cat(watch_path, ":watch");
	else
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
	watch_path = sds_cat(watch_path, "/watcher-");
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
	zvalue_t *value, *watch_value;
	ztree_t *ztree;
	int count, i;

	ztree = zserver->ztree;
	watch_value = ztree->op->find_znode(ztree, path);
	if(watch_value == NULL)
	{
		return ZNO_EXISTS;
	}

	watch_path = sds_dup(path);
	for(i = sds_len(watch_path) - 1; i >= 0 && watch_path[i] != ':'; i--);
	if(i < 0)
	{
		watch_path = sds_cat(watch_path, ":watch");
	}
	else
	{
		watch_path = sds_cat(watch_path, "/watch");
	}
	value = ztree->op->find_znode(ztree, watch_path);
	if(value == NULL)
	{
		sds_free(watch_path);
		destroy_zvalue(watch_value);
		return ZOK;
	}
	watch_child = ztree->op->get_children(ztree, watch_path, &count);
	destroy_zvalue(value);
	watcher = sds_dup(watch_path);
	for(i = 0; i < count; i++)
	{
		watcher = sds_cpy(watcher, watch_path);
		watcher = sds_cat(watcher, "/");
		watcher = sds_cat(watcher, watch_child[i]);
		value = ztree->op->find_znode(ztree, watcher);
		//notice this child
		if(notice_child(zserver, value, notice_type, watch_value) != ZOK)
		{
			destroy_zvalue(value);
			continue;
		}
		destroy_zvalue(value);
		ztree->op->delete_znode(ztree, watcher);
	}

	sds_free_split_res(watch_child, count);
	sds_free(watcher);
	destroy_zvalue(watch_value);
	sds_free(watch_path);
	return ZOK;
}

//===================ABOUT OPERATING ZNODE FUNCTIONS=====================
static int create_znode(zserver_t *zserver, const sds path, const sds data, znode_type_t type, sds return_name)
{
	zvalue_t *zvalue;
	ztree_t *ztree;

	zvalue = create_zvalue(data, type, 1);
	if(zvalue == NULL)
		return ZCREATE_WRONG;
	ztree = zserver->ztree;
	if(ztree->op->add_znode(ztree, path, zvalue, return_name) == -1)
	{
		destroy_zvalue(zvalue);
		return ZCREATE_WRONG;
	}
	destroy_zvalue(zvalue);
	return ZOK;
}

static int create_parent(zserver_t *zserver, const sds path, const sds data, znode_type_t type, sds return_name)
{
	zvalue_t *zvalue;
	ztree_t *ztree;

	zvalue = create_zvalue_parent(data, type, 1);
	if(zvalue == NULL)
	{
		return ZCREATE_WRONG;
	}
	ztree = zserver->ztree;
	if(ztree->op->add_znode(ztree, path, zvalue, return_name) == -1)
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
	if(version == -1 || version == zvalue->status.version)
	{
		old_data = zvalue->data;
		zvalue->data = sds_dup(data);
		zvalue->update_znode_status(&zvalue->status, (zvalue->status.version + 1) % MAX_VER_NUM, ZNODE_MODIFY);
		sds_free(old_data);
		notify_watcher(zserver, path, NOTICE_CHANGED);
		destroy_zvalue(zvalue);
		return ZOK;
	}
	destroy_zvalue(zvalue);
	return ZWRONG_VERSION;
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
	value->update_znode_status(&value->status, -1, ZNODE_ACCESS);
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
static int get_znode(zserver_t *zserver, const sds path, sds return_data,
		znode_status_t *status, sds watch_data)
{
	zvalue_t *value;
	ztree_t *ztree;

	ztree = zserver->ztree;
	value = ztree->op->find_znode(ztree, path);
	if(value == NULL)
		return ZNO_EXISTS;

	//get znode data
	value->update_znode_status(&value->status, -1, ZNODE_ACCESS);
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
	{
		return ZOK;
	}
	if(add_watcher(zserver, path, watch_data) != ZOK)
	{
		return ZOK | ZSET_WATCH_ERROR;
	}
	return ZOK;
}

static int get_children(zserver_t *zserver, sds path, sds return_data)
{
	ztree_t *ztree;
	int count, i;
	sds *children = NULL;

	ztree = zserver->ztree;
	children = ztree->op->get_children(ztree, path, &count);
	if(children == NULL)
		return ZNO_CHILDREN;
	return_data = sds_cpy(return_data, children[0]);

	for(i = 1; i < count; i++)
	{
		return_data = sds_cat(return_data, " ");
		return_data = sds_cat(return_data, children[i]);
	}
	sds_free_split_res(children, count);
	return ZOK;
}

//============================FOR DEBUG==================================
static void printf_sim(zreturn_sim_t *zreturn)
{
	log_write(LOG_DEBUG, "return code is %d, return value is %s",
			zreturn->return_code, zreturn->data);
}

static void printf_base(zreturn_base_t *zreturn)
{
	log_write(LOG_DEBUG, "return code is %d", zreturn->return_code);
}

static void printf_complex(zreturn_complex_t *zreturn)
{
	log_write(LOG_DEBUG, "return code is %d, return value is %s",
			zreturn->return_code, zreturn->data);
	log_write(LOG_DEBUG, "The status is: version: %d, create time: %llu, modified time: %llu, access time: %llu",
			zreturn->status.version, zreturn->status.created_time,
			zreturn->status.modified_time, zreturn->status.access_time);
}

static void printf_mid(zreturn_mid_t *zreturn)
{
	log_write(LOG_DEBUG, "return code is %d",
			zreturn->return_code);
	log_write(LOG_DEBUG, "The status is: version: %d, create time: %llu, modified time: %llu, access time: %llu",
			zreturn->status.version, zreturn->status.created_time,
			zreturn->status.modified_time, zreturn->status.access_time);
}

//==========================RESOLVE HANDLER==============================
static int init_znode_handler(event_handler_t *event_handler, zserver_t
		*zserver, void* common_msg)
{
	list_t *list;
	void *msg;

	event_handler->special_struct = zserver;
	event_handler->event_buffer_list = list_create();
	if( (list = event_handler->event_buffer_list) == NULL )
	{
		log_write(LOG_ERR, "error when allocate list");
		return -1;
	}
	list_set_free_method(list, zfree);
	msg = zmalloc(sizeof(common_msg_t));
	common_msg_dup(msg, common_msg);
	list->list_ops->list_add_node_head(list, msg);
	return 0;
}

static void create_znode_handler(event_handler_t *event_handler)
{
	sds path, data, return_name;
	zserver_t *zserver = event_handler->special_struct;
	list_t *list = event_handler->event_buffer_list;
	common_msg_t* common_msg = list_node_value(list->list_ops->list_index(list,
				0));
	zoo_create_znode_t *zmsg = (void *)MSG_COMM_TO_CMD(common_msg);
	int return_code, source;
	zreturn_sim_t* zreturn = zmalloc(sizeof(zreturn_sim_t));
	rpc_server_t *rpc_server = zserver->rpc_server;

	source = common_msg->source;
	path = sds_new((const char *)zmsg->path);
	data = sds_new((const char *)zmsg->data);
	return_name = sds_new_len(NULL, MAX_RET_DATA_LEN);
	return_code = create_znode(zserver, path, data, zmsg->znode_type, return_name);

	zreturn->return_code = return_code;
	memset(zreturn->data, 0, sizeof(zreturn->data));
	if(return_code == ZOK)
		memcpy(zreturn->data, return_name, sds_len(return_name));

	rpc_server->op->send_to_queue(rpc_server, zreturn, source,
			zmsg->unique_tag, sizeof(zreturn_sim_t));
#if ZSERVER_DEBUG
	printf_sim(zreturn);
#endif

	sds_free(path);
	sds_free(data);
	sds_free(return_name);
	zfree(zreturn);
	list_release(list);
}

static void create_parent_handler(event_handler_t *event_handler)
{
#if ZSERVER_DEBUG
	log_write(LOG_DEBUG, "zserver create parent handler start");
#endif
	sds path, data, return_name;
	zserver_t *zserver = event_handler->special_struct;
	list_t *list = event_handler->event_buffer_list;
	common_msg_t* common_msg = list_node_value(list->list_ops->list_index(list, 0));
	zoo_create_znode_t *zmsg = (void *)MSG_COMM_TO_CMD(common_msg);
	int return_code, source;
	zreturn_sim_t* zreturn = zmalloc(sizeof(zreturn_sim_t));
	rpc_server_t *rpc_server = zserver->rpc_server;

	source = common_msg->source;
	path = sds_new((const char *)zmsg->path);
	data = sds_new((const char *)zmsg->data);
	return_name = sds_new_len(NULL, MAX_RET_DATA_LEN);
	return_code = create_parent(zserver, path, data, zmsg->znode_type, return_name);

#if ZSERVER_DEBUG
	log_write(LOG_DEBUG, "zserver create parent handler execute create parent success");
#endif
	zreturn->return_code = return_code;
	memset(zreturn->data, 0, sizeof(zreturn->data));
	if(return_code == ZOK)
	{
		memcpy(zreturn->data, return_name, sds_len(return_name));
	}

	rpc_server->op->send_to_queue(rpc_server, zreturn, source, zmsg->unique_tag, sizeof(zreturn_sim_t));
#if ZSERVER_DEBUG
	printf_sim(zreturn);
#endif

	sds_free(path);
	sds_free(data);
	sds_free(return_name);
	zfree(zreturn);
	list_release(list);
}

static void delete_znode_handler(event_handler_t *event_handler)
{
#if ZSERVER_DEBUG
log_write(LOG_DEBUG, "start delete znode");
#endif
	sds path;
	zserver_t *zserver = event_handler->special_struct;
	list_t *list = event_handler->event_buffer_list;
	common_msg_t* common_msg = list_node_value(list->list_ops->list_index(list, 0));
	zoo_delete_znode_t *zmsg = (void *)MSG_COMM_TO_CMD(common_msg);
	int return_code, source;
	zreturn_base_t* zreturn = zmalloc(sizeof(zreturn_base_t));
	rpc_server_t *rpc_server = zserver->rpc_server;

	source = common_msg->source;
	path = sds_new((const char *)zmsg->path);
	if(zmsg->version == (uint16_t) - 1)
	{
		return_code =  delete_znode(zserver, path, -1);
	}
	else
	{
		return_code =  delete_znode(zserver, path, zmsg->version);
	}
#if ZSERVER_DEBUG
	log_write(LOG_DEBUG, "delete znode handler send zreturn code to queue");
#endif
	zreturn->return_code = return_code;
	rpc_server->op->send_to_queue(rpc_server, zreturn, source, zmsg->unique_tag, sizeof(zreturn_base_t));
#if ZERVER_DEBUG
	printf_base(zreturn);
#endif

	sds_free(path);
	zfree(zreturn);
	list_release(list);
}

static void set_znode_handler(event_handler_t *event_handler)
{
	sds path, data;
	zserver_t *zserver = event_handler->special_struct;
	list_t *list = event_handler->event_buffer_list;
	common_msg_t* common_msg = list_node_value(list->list_ops->list_index(list,
				0));
	zoo_set_znode_t *zmsg = (void *)MSG_COMM_TO_CMD(common_msg);
	int return_code, source;
	zreturn_base_t* zreturn = zmalloc(sizeof(zreturn_base_t));
	rpc_server_t *rpc_server = zserver->rpc_server;

	source = common_msg->source;
	path = sds_new((const char *)zmsg->path);
	data = sds_new((const char *)zmsg->data);
	if(zmsg->version == (uint16_t)-1)
		return_code = set_znode(zserver, path, data, -1);
	else
		return_code = set_znode(zserver, path, data, zmsg->version);

	zreturn->return_code = return_code;

	rpc_server->op->send_to_queue(rpc_server, zreturn, source,
			zmsg->unique_tag, sizeof(zreturn_base_t));
#if ZSERVER_DEBUG
	printf_base(zreturn);
#endif

	sds_free(path);
	sds_free(data);
	zfree(zreturn);
	list_release(list);
}

static void get_znode_handler(event_handler_t *event_handler)
{
	sds path, return_data, watch_data;
	zserver_t *zserver = event_handler->special_struct;
	list_t *list = event_handler->event_buffer_list;
	common_msg_t *common_msg = list_node_value(list->list_ops->list_index(list, 0));
	zoo_get_znode_t *zmsg = (void *)MSG_COMM_TO_CMD(common_msg);
	int return_code, source;
	zreturn_complex_t *zreturn = zmalloc(sizeof(zreturn_complex_t));
	rpc_server_t *rpc_server = zserver->rpc_server;
	char string[50];

	source = common_msg->source;
	path = sds_new((const char *)zmsg->path);
	return_data = sds_new_len(NULL, MAX_RET_DATA_LEN);
	if(zmsg->watch_flag != 0)
	{
		sprintf(string, "%d %d %d %d", source, WATCH_TAG, zmsg->watch_flag, zmsg->watch_code);
		watch_data = sds_new(string);
	}
	else
		watch_data = NULL;
	return_code = get_znode(zserver, path, return_data, &zreturn->status, watch_data);

	zreturn->return_code = return_code;
	memset(zreturn->data, 0, sizeof(zreturn->data));
	if(return_code & ZOK)
	{
		memcpy(zreturn->data, return_data, sds_len(return_data));
	}

	rpc_server->op->send_to_queue(rpc_server, zreturn, source, zmsg->unique_tag, sizeof(zreturn_complex_t));
#if ZERVER_DEBUG
	printf_complex(zreturn);
#endif

	sds_free(path);
	sds_free(return_data);
	sds_free(watch_data);
	zfree(zreturn);
	list_release(list);
}

static void get_children_handler(event_handler_t *event_handler)
{
	sds path, return_data;
	zserver_t *zserver = event_handler->special_struct;
	list_t *list = event_handler->event_buffer_list;
	common_msg_t *common_msg = list_node_value(list->list_ops->list_index(list, 0));
	zoo_get_children_t *zmsg = (void*)MSG_COMM_TO_CMD(common_msg);
	int return_code, source;
	zreturn_sim_t *zreturn = zmalloc(sizeof(zreturn_sim_t));
	rpc_server_t *rpc_server = zserver->rpc_server;

	source = common_msg->source;
	path = sds_new((const char*)zmsg->path);
	return_data = sds_new_len(NULL, MAX_RET_DATA_LEN);
	return_code = get_children(zserver, path, return_data);

	zreturn->return_code = return_code;
	memset(zreturn->data, 0, sizeof(zreturn->data));
	if(return_code == ZOK)
		memcpy(zreturn->data, return_data, sds_len(return_data));

	rpc_server->op->send_to_queue(rpc_server, zreturn, source, zmsg->unique_tag, sizeof(zreturn_sim_t));
#if ZSERVER_DEBUG
	printf_sim(zreturn);
#endif

	sds_free(path);
	sds_free(return_data);
	zfree(zreturn);
	list_release(list);
}

static void exists_znode_handler(event_handler_t *event_handler)
{
	sds path, watch_data;
	zserver_t *zserver = event_handler->special_struct;
	list_t *list = event_handler->event_buffer_list;
	common_msg_t* common_msg = list_node_value(list->list_ops->list_index(list,
				0));
	zoo_get_znode_t *zmsg = (void *)MSG_COMM_TO_CMD(common_msg);
	int return_code, source;
	zreturn_mid_t *zreturn = zmalloc(sizeof(zreturn_mid_t));
	rpc_server_t *rpc_server = zserver->rpc_server;
	char string[50];

	source = common_msg->source;
	path = sds_new((const char *)zmsg->path);
	if(zmsg->watch_flag != 0)
	{
		sprintf(string, "%d %d %d %d", source, WATCH_TAG, zmsg->watch_flag, zmsg->watch_code);
		watch_data = sds_new(string);
	}
	else
		watch_data = NULL;
	return_code = exists_znode(zserver, path, &zreturn->status, watch_data);

	zreturn->return_code = return_code;

	rpc_server->op->send_to_queue(rpc_server, zreturn, source,
			zmsg->unique_tag, sizeof(zreturn_mid_t));
#if ZERVER_DEBUG
	printf_mid(zreturn);
#endif

	sds_free(path);
	sds_free(watch_data);
	zfree(zreturn);
	list_release(list);
}

static void *resolve_handler(event_handler_t *event_handler, void *msg_queue)
{
#if ZSEVER_DEBUG
	puts("RECEIVE+++++++++++++++++MESSAGR");
#endif
	static common_msg_t common_msg;
	syn_queue_t *queue = msg_queue;

	queue->op->syn_queue_pop(queue, &common_msg);
	switch(common_msg.operation_code)
	{
		case ZOO_CREATE_CODE:
			init_znode_handler(event_handler, local_zserver, &common_msg);
			event_handler->handler = create_znode_handler;
			break;
		case ZOO_CREATE_PARENT_CODE:
			init_znode_handler(event_handler, local_zserver, &common_msg);
			event_handler->handler = create_parent_handler;
			break;
		case ZOO_GET_CODE:
			init_znode_handler(event_handler, local_zserver, &common_msg);
			event_handler->handler = get_znode_handler;
			break;
		case ZOO_GET_CHILDREN_CODE:
			init_znode_handler(event_handler, local_zserver, &common_msg);
			event_handler->handler = get_children_handler;
			break;
		case ZOO_SET_CODE:
			init_znode_handler(event_handler, local_zserver, &common_msg);
			event_handler->handler = set_znode_handler;
			break;
		case ZOO_EXISTS_CODE:
			init_znode_handler(event_handler, local_zserver, &common_msg);
			event_handler->handler = exists_znode_handler;
			break;
		case ZOO_DELETE_CODE:
			init_znode_handler(event_handler, local_zserver, &common_msg);
			event_handler->handler = delete_znode_handler;
			break;
		default:
			event_handler->handler = NULL;
			break;
	}
	return event_handler->handler;
}

//==========================ZSERVER OPERATIONS=========================
static void zput_request(zserver_t *zserver, common_msg_t *common_msg)
{
	syn_queue_t *msg_queue;
	
	msg_queue = zserver->rpc_server->request_queue;
	msg_queue->op->syn_queue_push(msg_queue, common_msg);
}

static void zserver_start(zserver_t *zserver)
{
	rpc_server_t *rpc_server = zserver->rpc_server;

	rpc_server->op->server_start2(rpc_server);
}

static void zserver_stop(zserver_t *zserver)
{
	rpc_server_t *rpc_server = zserver->rpc_server;

	rpc_server->op->server_stop2(rpc_server);
}
//===========================PUBLIC FUNCTIONS===========================
zserver_t *create_zserver(int server_id)
{
	zserver_t *zserver;

	zserver = zmalloc(sizeof(zserver_t));
	assert(zserver != NULL);

	zserver->op = zmalloc(sizeof(zserver_op_t));
	zserver->op->zserver_start = zserver_start;
	zserver->op->zserver_stop = zserver_stop;
	zserver->op->zput_request = zput_request;

	zserver->ztree = create_ztree(1);
	zserver->rpc_server = create_rpc_server2(1, RECV_QUEUE_SIZE,
			SEND_QUEUE_SIZE, server_id, resolve_handler);
	//set stastic server
	local_zserver = zserver;
	return zserver;
}

void destroy_zserver(zserver_t *zserver)
{
	destroy_rpc_server(zserver->rpc_server);
	destroy_ztree(zserver->ztree);
	zfree(zserver->op);
	zfree(zserver);
}
