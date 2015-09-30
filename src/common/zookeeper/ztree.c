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
#include <stdlib.h>
#include "zookeeper.h"
#include "zmalloc.h"
#include "sds.h"
#include "log.h"

//==============================PRIVATE FUNCTION DECLARE==========================
static void update_znode_status(znode_status_t *status, int version, int mode);
static sds add_seq(zvalue_t *value, sds node_name);

static void *pair_dup(const void *pair);
static void pair_free(void* pair);
static void v_free(void *v);
static void *v_dup(const void *v);

static zvalue_t *find_znode(ztree_t *tree, const sds path);
static int add_znode(ztree_t *tree, const sds path, zvalue_t *value, sds return_name);
static void delete_znode(ztree_t *tree, const sds path);
static sds *get_children(ztree_t *tree, const sds path, int *count);

//================================================================================
static sds add_seq(zvalue_t *value, sds node_name)
{
	int seq;
	sds number;

	//noly one thread execute it, so do not need mutex
	seq = (value->seq++) % SEQUENCE_MAX;

	number = sds_new_ull(seq);
	node_name = sds_cat_sds(node_name, number);
	sds_free(number);
	return node_name;
}

static void update_znode_status(znode_status_t *status, int version, int mode)
{
	if(version > 0)
		status->version = version;
	else
		status->version = (status->version % MAX_VER_NUM);
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
zvalue_t *create_zvalue(const sds data, znode_type_t type, int version)
{
	int return_value;
	zvalue_t *this = zmalloc(sizeof(zvalue_t));
	if(this == NULL)
		return NULL;
	this->child = NULL;
	this->type = type;
	if(data != NULL)
		this->data = sds_dup(data);
	else
		this->data = NULL;
	this->seq = 1;
	this->reference = 1;
	this->update_znode_status = update_znode_status;
	return_value = pthread_mutex_init(&this->zvalue_lock, NULL);
	assert(return_value == 0);
	update_znode_status(&(this->status), version, ZNODE_CREATE);

	return this;
}

zvalue_t *create_zvalue_parent(const sds data, znode_type_t type, int version)
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

void zstatus_dup(znode_status_t *dst, const znode_status_t *src)
{
	dst->version = src->version;
	dst->access_time = src->access_time;
	dst->modified_time = src->modified_time;
	dst->created_time = src->created_time;
}

void destroy_zvalue(zvalue_t *value)
{
	if(value == NULL)
		return;
	pthread_mutex_lock(&value->zvalue_lock);
	if(--value->reference == 0)
	{
		pthread_mutex_unlock(&value->zvalue_lock);
		pthread_mutex_destroy(&value->zvalue_lock);
		sds_free(value->data);
		//you should make true that no more refences on this zvalue add its' child
		if(value->child != NULL)
			destroy_map(value->child);
		value->data = NULL;
		value->reference = -1;
		value->child = NULL;
		zfree(value);
		value = NULL;
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
	p->value = zvalue_dup(((pair_t *)pair)->value);
	return (void *)p;
}

static void pair_free(void *pair)
{
	pair_t *p = pair;
	sds_free(p->key);
	destroy_zvalue(p->value);
	zfree(p);
}

static void v_free(void *v)
{
	zvalue_t *value = v;
	destroy_zvalue(value);
}

static void *v_dup(const void *v)
{
	zvalue_t *value = (void *)v;
	return (void *)zvalue_dup(value);
}

//==================================ZTREE FUNCTIONS================================
//this function will going to find correct zvalue related by path and will
//return NULL if this zvalue does not exist. The path format will be like
///temp/tmp1:watch/watch1. Before ':' is file path, represent real file stored
//in this server and behind ':' is created by client whitch is used to
//synchronize the requests, if node type is squential, path should be like
///temp/tmp1:watch/watch-, otherwise, path should not contain '-'
static zvalue_t *find_znode(ztree_t *tree, const sds path)
{
	int count, sub_count, i;
	sds file_path, zvalue_path;
	zvalue_t *file_node, *sub_node = NULL;
	map_t *zpath, *sub_path;
	sds *args, *sub_args;
	args = sds_split_len(path, sds_len(path), ":", 1, &count);

	//path is like /temp/tmp1:
	zpath = tree->zpath;
	if(count == 1)
	{
		file_path = args[0];
		file_node = zpath->op->get(zpath, file_path);
		sds_free_split_res(args, count);
		return file_node;
	}
	file_path = args[0];
	zvalue_path = args[1];

	file_node = zpath->op->get(zpath, file_path);
	if(file_node == NULL)
	{
		sds_free_split_res(args, count);
		return NULL;
	}

	//find file path node, now need to find sub path node
	sub_path = file_node->child;
	destroy_zvalue(file_node);
	sub_args = sds_split_len(zvalue_path, sds_len(zvalue_path), "/", 1, &sub_count);
	sds_free_split_res(args, count);

	if(sub_path == NULL)
	{
		sds_free_split_res(sub_args, sub_count);
		return NULL;
	}
	for(i = 0; i < sub_count; i++)
	{
		if(sub_node != NULL)
			destroy_zvalue(sub_node);
		sub_node = sub_path->op->get(sub_path, sub_args[i]);
		if(sub_node == NULL)
		{
			sds_free_split_res(sub_args, sub_count);
			return NULL;
		}
		sub_path = sub_node->child;
	}
	sds_free_split_res(sub_args, sub_count);
	//this reference send to caller, and caller need to destroy it
	return sub_node;
}

//This function will add znode to ztree success return 0, failed return -1,
//value in args list should be destroyed by caller
//return_name should be long enough to got whole name
static int add_znode(ztree_t *tree, const sds path, zvalue_t *value, sds return_name)
{
	sds parent_path, node_name;
	int len, start = 0, end;
	zvalue_t *parent_node;
	map_t *child_map = NULL;
	char reserve;

	len = sds_len(path);
	parent_path = sds_dup(path);
	//if the path is like "/temp/tmp"
	for(end = len - 1; end >= 0 && parent_path[end] != ':'; end--);
	if(end < 0)
	{
		map_t *child_map = tree->zpath;
		child_map->op->put(child_map, parent_path, value);
		log_write(LOG_DEBUG, "Add znode: The key is %s", parent_path);
		sds_free(parent_path);
		if(return_name != NULL)
			return_name = sds_cpy(return_name, path);
		return 0;
	}

	for(end = len - 1; parent_path[end] != '/' && parent_path[end] != ':'; end--);
	assert(end > 0);
	reserve = parent_path[end];
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
	//change node name if type contain squential
	if(value->type == PERSISTENT_SQUENTIAL || value->type == EPHEMERAL_SQUENTIAL)
		node_name = add_seq(parent_node, node_name);
	child_map->op->put(child_map, node_name, value);
	log_write(LOG_DEBUG, "Add znode: The dir is %s, the key is %s",
			parent_path, node_name);
	if(return_name != NULL)
	{
		return_name = sds_cpy(return_name, parent_path);
		if(reserve == ':')
			return_name = sds_cat(return_name, ":");
		else
			return_name = sds_cat(return_name, "/");
		return_name = sds_cat(return_name, node_name);
	}

	destroy_zvalue(parent_node);
	sds_free(parent_path);
	sds_free(node_name);

	return 0;
}

static void delete_znode(ztree_t *tree, const sds path)
{
	sds parent_path, node_name;
	int len, start = 0, end;
	zvalue_t *parent_node;
	map_t *child_map = NULL;

	len = sds_len(path);
	parent_path = sds_dup(path);
	//if the path is like "/temp/tmp"
	for(end = len - 1; end >= 0 && parent_path[end] != ':'; end--);
	if(end < 0)
	{
		map_t * child_map = tree->zpath;
		child_map->op->del(child_map, parent_path);
		sds_free(parent_path);
		return;
	}

	for(end = len - 1; parent_path[end] != '/' && parent_path[end] != ':'; end--);
	assert(end > 0);
	sds_range(parent_path, start, end - 1);
	node_name = sds_dup(path);
	sds_range(node_name, end + 1, -1);

	parent_node = find_znode(tree, parent_path);
	if(parent_node == NULL)
		return;
	child_map = parent_node->child;
	child_map->op->del(child_map, node_name);
	//update parent znode status
	update_znode_status(&parent_node->status, parent_node->status.version + 1,
			ZNODE_MODIFY);

	destroy_zvalue(parent_node);
	sds_free(parent_path);
	sds_free(node_name);
}

//This function will return children of given parent path, the path of children
//will return into a sds array, and it will return NULL if there is no child.
static sds *get_children(ztree_t *tree, const sds path, int *count)
{
	zvalue_t *parent_node;
	map_t *map;
	sds *path_array;

	parent_node = find_znode(tree, path);
	if(parent_node == NULL)
		return NULL;
	map = parent_node->child;
	if(map == NULL)
		return NULL;
	path_array = map->op->get_all_keys(map, count);
	destroy_zvalue(parent_node);
	return path_array;
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
	this->op->get_children = get_children;

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
