/* zookeeper.h
 * created on 2015.9.10
 * author: Binyang
 * This file is going to create a basic tool to coordinate each client
 * operations, just like zookeeper.
 * This tool will provide four major functoins
 * 1. you can use this tool to achieve write lock and read-write lock;
 * 2. visit and change meta-data will be atomistic, when one client change
 *    meta-data, other client has to wait;
 * 3. change will be ordered, the change will be excuse by the order they arrive
 *    server. You should provide version number if you want server to change
 *    data after certain version;
 * 4. client can set watch flag. If a client want to be notified when a data
 *    changed, it can set watch flag to this data. The watch mechanism will only
 *    be triggered once after flag setted.
 */
#ifndef _ZOOKEEPER_H_
#define _ZOOKEEPER_H_
#include <stdint.h>
#include <pthread.h>
#include "sds.h"
#include "map.h"
#include "basic_list.h"
#include "rpc_server.h"
#include "rpc_client.h"

#define ZNODE_CREATE 0x01
#define ZNODE_MODIFY 0x02
#define ZNODE_ACCESS 0x04
//number of zpath stored in ztree
#define ZPATH_COUNT 128
#define ZVALUE_CHILD_COUNT 16
#define SEQUENCE_MAX 1048576
//zserver define
#define RECV_QUEUE_SIZE 128
#define SEND_QUEUE_SIZE 64

struct znode_status
{
	uint64_t created_time;
	uint64_t modified_time;
	uint64_t access_time;
	int version;
};

enum znode_type
{
	PERSISTENT = 1,
	EPHEMERAL = 2,
	PERSISTENT_SQUENTIAL = 3,
	EPHEMERAL_SQUENTIAL = 4
};

struct zvalue
{
	sds data;
	struct znode_status status;
	enum znode_type type;
	map_t *child;
	int reference;
	int seq;
	pthread_mutex_t zvalue_lock;
};

struct ztree;

struct ztree_op
{
	struct zvalue *(*find_znode)(struct ztree *tree, sds path);
	int (*add_znode)(struct ztree *tree, sds path, struct zvalue *value, sds
			return_name);
	void (*delete_znode)(struct ztree *tree, sds path);
	sds *(*get_children)(struct ztree *tree, sds path, int *count);
};

struct ztree
{
	map_t *zpath;
	struct ztree_op *op;
	struct znode_status status;
};

struct zserver_op
{
	void (*create_parent)();
	void (*create_znode)();
	void (*delete_znode)();
	void (*set_znode)();
	void (*get_znode)();
	void (*notify_watcher)();
	void (*start)();
	void (*stop)();
};

struct zserver
{
	struct zserver_op *op;
	struct ztree *ztree;
	rpc_server_t *rpc_server;
};


struct zclient_op
{
	void (*set_data)();
	void (*get_data)();
	void (*create_znode)();
	void (*delete_znode)();
};

struct zclient
{
	int client_id;
	struct zclient_op *op;
};

typedef struct znode_status znode_status_t;
typedef enum znode_type znode_type_t;
typedef struct zvalue zvalue_t;
typedef struct ztree ztree_t;
typedef struct ztree_op ztree_op_t;
typedef struct zserver_op zserver_op_t;
typedef struct zserver zserver_t;
typedef struct zclient_op zclient_op_t;
typedef struct zclient zclient_t;

zvalue_t *create_zvalue(sds data, znode_type_t type, int version);
zvalue_t *create_zvalue_parent(sds data, znode_type_t type, int version);
zvalue_t *zvalue_dup(zvalue_t *value);
void destroy_zvalue(zvalue_t *zvalue);

ztree_t *create_ztree(int version);
void destroy_ztree(ztree_t *tree);

zserver_t *create_zserver(int server_id);
#endif
