/*
 * author: Binyang
 * created on: 2015.9.10
 * This file is going to finish zookeeper tree structure functions whitch defined
 * in zookeeper.h
 */
#include <pthread.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include "zookeeper.h"
#include "zmalloc.h"
#include "sds.h"
#include "log.h"

//==============================PRIVATE FUNCTION DECLARE==========================
static void update_znode_status(znode_status_t *status, int version, int mode);

static void *pair_dup(const void *pair);
static void pair_free(void* pair);
static void v_free(void *v);
static void *v_dup(const void *v);

static zvalue *find_znode(ztree_t *tree, sds path);
static int add_znode(ztree_t *tree, sds path, zvalue *value);
static void delete_znode(ztree_t *tree, sds path);

static void update_znode_status(znode_status_t *status, int version, int mode)
{
	if(version > 0)
		status->version = version;
	else
		status->version = -1;
	switch(mode)
	{
		case ZNODE_CREATE:
			status->modified_time = status->access_time =
				status->created_time = time(NULL);
			break;
		case ZNODE_MODIFY:
			status->modified_time = status->access_time = time(NULL);
			break;
		case ZNODE_ACCESS:
			status->access_time = time(NULL);
			break;
		default:
			return;
	}
}

//===============================ZVALUE FUNCTIONS==================================
zvalue_t *create_zvalue(sds data, znode_type_t type, int version)
{
	int return_value;
	zvalue_t *this = zmalloc(sizeof(zvalue_t));
	if(this == NULL)
		return NULL;
	this->child = NULL;
	this->type = type;
	this->data = sds_dup(data);
	this->reference = 1;
	return_value = pthread_mutex_init(&this->zvalue_lock, NULL);
	assert(return_value == 0);
	update_znode_statue(&(this->status), version, ZNODE_CREATE);

	return this;
}

zvalue_t *create_zvalue_parent(sds data, znode_type_t type, int version)
{
	zvalue_t *this = create_zvalue(data, type, version);
	if(this == NULL)
		return NULL;
	this->child = create_map(ZVALUE_CHILD_COUNT, v_dup, v_free, pair_dup, pair_free);
	return this;
}

zvalue_t *zvalue_dup(zvalue_t *value)
{
	pthread_mutex_lock(&value->zvalue_lock);
	value->reference++;
	pthread_mutex_unlock(&value->zvalue_lock);
	return value;
}

void destroy_zvalue(zvalue_t *value)
{
	pthread_mutex_lock(&value->zvalue_lock);
	if(--value->reference == 0)
	{
		pthread_mutex_unlock(&value->zvalue_lock);
		pthread_mutex_destroy(&value->zvalue_lock);
		sds_free(value->data);
		//you should make true that no more refences on this zvalue add its' child
		if(value->child != NULL)
			destroy_map(value->child);
		zfree(value);
	}
	else
	{
		pthread_mutex_unlock(&value->zvalue_lock);
	}
}

//==============================USED FOR CREATING ZPATH============================
static void *pair_dup(const void *pair)
{
	pair_t *p = zmalloc(sizeof(pair_t));
	p->key = sds_dup(((pair_t *)pair)->key);
	p->value = zvalue_dup(pair->value);
	return (void *)p;
}

static void pair_free(void *pair)
{
	pair_t *p = pair;
	sds_free(p->key);
	destroy_zvalue(p->value);
	zfree(p);
}

static v_free(void *v)
{
	zvalue_t *value = v;
	destroy(value);
}

static void *v_dup(const void *v)
{
	zvalue_t *value = v;
	return (void *)zvalue_dup(value);
}

//==================================ZTREE FUNCTIONS================================
//this function will going to find correct zvalue related by path and will
//return NULL if this zvalue does not exist. The path format will be like
///temp/tmp1:watch/watch1. Before ':' is file path, represent real file stored
//in this server and behind ':' is created by client whitch is used to
//synchronize the requests
static zvalue *find_znode(ztree_t *tree, sds path)
{
	int count, sub_count, i;
	sds file_path, zvalue_path;
	zvalue_t *file_node, sub_node = NULL;
	map_t *zpath, sub_path;
	sds *args, *sub_args;
	args = sds_split_len(path, sds_len(path), ":", &count);

	//path is like /temp/tmp1:
	if(count == 1)
	{
		file_path = args[0];
		file_node = zpath->op->get(zpath, file_path);
		sds_free_split_res(args, count);
		return file_node;
	}
	file_path = args[0];
	zvalue_path = args[1];

	zpath = tree->zpath;
	file_node = zpath->op->get(zpath, file_path);
	if(file_node == NULL)
	{
		sds_free_split_res(args, count);
		return NULL;
	}

	//find file path node, now need to find sub path node
	sub_path = file_node->child;
	destroy_zvalue(file_node);
	sub_args = sds_split_len(zvalue_path, sds_len(zvalue_path), "\\", &sub_count);
	sds_free_split_res(args, count);

	for(i = 0; i < sub_count; i++)
	{
		if(sub_node != NULL)
			destroy_zvalue(sub_node);
		sub_node = sub_path->op->get(sub_path, sub_args[i]);
		if(sub_node == NULL)
		{
			destroy_zvalue(sub_node);
			sds_free_split_res(sub_args, sub_count);
			return NULL;
		}
		sub_path = sub_node->child;
	}
	//this reference send to caller, and caller need to destroy it
	return sub_node;
}

//This function will add znode to ztree success return 0, failed return -1,
//value in args list should be destroyed by caller
static int add_znode(ztree_t *tree, sds path, zvalue *value)
{
	sds parent_path, node_name;
	int len, start = 0, end;
	zvalue *parent_node;
	map_t *child_map = NULL;

	len = sds_len(path);
	parent_path = sds_dup(path);
	//if the path is like "/temp/tmp"
	for(end = len - 1; end >= 0 && parent_path[end] != ':', end--);
	if(end < 0)
	{
		map_t *child_map = tree->zpath;
		child_map->op->put(child_map, parent_path, value);
		sds_free(parent_path);
		return 0;
	}

	for(end = len - 1; parent_path[end] != '\\', end--);
	sds_range(parent_path, start, end - 1);
	node_name = sds_dup(path);
	sds_range(node_name, end + 1, -1);

	parent_node = find_znode(tree, parent_path);
	if(parent_node == NULL)
	{
		log_write(LOG_ERR, "path is incorrect");
		return -1;
	}
	child_map = parent_node->child;
	child_map->op->put(child_map, node_name, value);

	destroy_zvalue(parent_node);
	sds_free(parent_path);
	sds_free(node_name);
	return 0;
}

static void delete_znode(ztree_t *tree, sds path)
{
	sds parent_path, node_name;
	int len, start = 0, end;
	zvalue *parent_node;
	map_t *child_map = NULL;

	len = sds_len(path);
	parent_path = sds_dup(path);
	//if the path is like "/temp/tmp"
	for(end = len - 1; end >= 0 && parent_path[end] != ':', end--);
	if(end < 0)
	{
		map_t * child_map = tree->zpath;
		child_map->op->del(child_map, parent_path);
		sds_free(parent_path);
		return;
	}

	for(end = len - 1; parent_path[end] != '\\', end--);
	sds_range(parent_path, start, end - 1);
	node_name = sds_dup(path);
	sds_range(node_name, end + 1, -1);

	parent_node = find_znode(tree, parent_path);
	if(parent_path == NULL)
		return;
	child_map = parent_node->child;
	child_map->op->del(child_map, node_name);

	destroy_zvalue(parent_node);
	sds_free(parent_path);
	sds_free(node_name);
}

//================================public functions==============================
ztree_t *create_ztree(int version)
{
	ztree_t *this = zmalloc(sizeof(ztree_t));
	if(this == NULL)
		return NULL;
	this->zpath = create_map(ZPATH_COUNT, v_dup, v_free, pair_dup, pair_free);
	//init operations
	this->op = zmalloc(sizeof(ztree_op_t));
	this->op->find_znode = find_znode;
	this->op->add_znode = add_znode;
	this->op->delete_znode = delete_znode;

	update_znode_status(&this->status, version, ZNODE_CREATE);
	return this;
}

void destroy_ztree(ztree_t *tree)
{
	zfree(tree->op);
	//make sure that nodes in this tree has no refrences
	destroy_map(tree->zpath);
	zfree(tree);
}
