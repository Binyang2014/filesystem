/*
 * author: Binyang
 * created on: 2015.9.10
 * This file is going to finish zookeeper tree structure functions whitch defined
 * in zookeeper.h
 */
#include <pthread.h>
#include <time.h>
#include <assert.h>
#include "zookeeper.h"
#include "zmalloc.h"
#include "sds.h"

//==============================PRIVATE FUNCTION DECLARE==========================
static void update_znode_status(znode_status_t *status, int version, int mode);

static void* pair_dup(const void *pair);
static void pair_free(void* pair);
static void v_free(void *v);
static void* v_dup(const void *v);

static int add_znode();

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
static void* pair_dup(const void *pair)
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

static void* v_dup(const void *v)
{
	zvalue_t *value = v;
	return (void *)zvalue_dup(value);
}

//==================================ZTREE FUNCTIONS================================
ztree_t *create_ztree(int version)
{
	ztree_t *this = zmalloc(sizeof(ztree_t));
	if(this == NULL)
		return NULL;
	this->zpath = create_map(ZPATH_COUNT, v_dup, v_free, pair_dup, pair_free);
	this->op = zmalloc(sizeof(ztree_op_t));
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
