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
//zerror
#define ZOK 0x1
#define ZWRONG_VERSION 0x2
#define ZNO_EXISTS 0x4
#define ZCREATE_WRONG 0x8
#define ZSET_WATCH_ERROR 0x10
#define ZNO_ENOUGH_BUFFER 0x20
#define ZWRONG_WATCH_TYPE 0x40
#define ZRET_MSG_TOO_LONG 0x80
//znode create mode
#define ZNODE_CREATE 0x01
#define ZNODE_MODIFY 0x02
#define ZNODE_ACCESS 0x04
//number of zpath stored in ztree
#define ZPATH_COUNT 32
#define ZVALUE_CHILD_COUNT 8
#define SEQUENCE_MAX 512
#define MAX_VER_NUM 65535
//zserver define
#define RECV_QUEUE_SIZE 128
#define SEND_QUEUE_SIZE 64
//notice type
#define NOTICE_CHANGED 0x1
#define NOTICE_DELETE 0x2
//max return message data length
#define MAX_RET_DATA_LEN 128
//max watch code
#define MAX_WATCH_CODE 65535
//zclient define
#define ZCLIENT_RECV_QUEUE_SIZE 32

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
	void (*update_znode_status)(struct znode_status *status, int version, int
			mode);
	pthread_mutex_t zvalue_lock;
};

struct ztree;

struct ztree_op
{
	struct zvalue *(*find_znode)(struct ztree *tree, const sds path);
	int (*add_znode)(struct ztree *tree, const sds path, struct zvalue *value, sds
			return_name);
	void (*delete_znode)(struct ztree *tree, const sds path);
	sds *(*get_children)(struct ztree *tree, const sds path, int *count);
};

struct ztree
{
	map_t *zpath;
	struct ztree_op *op;
	struct znode_status status;
};

struct zserver;
struct zserver_op
{
	void (*zserver_start)(struct zserver *zserver);
	void (*zserver_stop)(struct zserver *zserver);
	void (*zput_request)(struct zserver *zserver, common_msg_t *common_msg);
};

struct zserver
{
	struct zserver_op *op;
	struct ztree *ztree;
	rpc_server_t *rpc_server;
};

struct zclient;
struct zclient_op
{
	int (*create_znode)(struct zclient*, const sds, const sds, enum  znode_type, sds);
	int (*create_parent)(struct zclient*, const sds, const sds, enum znode_type, sds);
	int (*delete_znode)(struct zclient*, const sds, int);
	int (*set_znode)(struct zclient*, const sds, const sds, int);
	int (*get_znode)(struct zclient*, const sds, sds, struct znode_status*, int,
			void*(*)(void*), void*);
	int (*exists_znode)(struct zclient*, const sds, struct znode_status*, int,
			void*(*)(void*), void*);
	void (*start_zclient)(struct zclient*);
	void (*stop_zclient)(struct zclient*);
};

struct zclient
{
	int client_id;
	int client_stop;
	struct zclient_op *op;

	rpc_client_t *rpc_client;
	void *send_buff;
	void *recv_buff;
	syn_queue_t *recv_queue;

	list_t *watch_list;
	uint16_t unique_watch_num;
	pthread_t watch_tid;
	pthread_t recv_watch_tid;
};

struct watch_data
{
	int dst;
	int tag;
	int watch_type;
	sds unique_code;//combine with client id and a unique number
};

struct zreturn_sim
{
	int return_code;
	char data[MAX_RET_DATA_LEN];
};

struct zreturn_complex
{
	int return_code;
	char data[MAX_RET_DATA_LEN];
	struct znode_status status;
};

struct zreturn_mid
{
	int return_code;
	struct znode_status status;
};

struct zreturn_base
{
	int return_code;
};

//ret data could not exceed 128 bytes
struct watch_ret_msg
{
	int watch_type;
	char ret_data[MAX_RET_DATA_LEN];
	struct znode_status status;
};

struct watch_node
{
	int watch_type;
	uint16_t watch_code;
	void* (*watch_handler)(void *);
	void* args;
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
typedef struct watch_data watch_data_t;
typedef struct watch_ret_msg watch_ret_msg_t;
typedef struct zreturn_sim zreturn_sim_t;
typedef struct zreturn_complex zreturn_complex_t;
typedef struct zreturn_base zreturn_base_t;
typedef struct zreturn_mid zreturn_mid_t;
typedef void *(*watch_handler_t)(void *);
typedef struct watch_node watch_node_t;

void zstatus_dup(znode_status_t *dst, const znode_status_t *src);
zvalue_t *create_zvalue(const sds data, znode_type_t type, int version);
zvalue_t *create_zvalue_parent(const sds data, znode_type_t type, int version);
zvalue_t *zvalue_dup(zvalue_t *value);
void destroy_zvalue(zvalue_t *zvalue);

ztree_t *create_ztree(int version);
void destroy_ztree(ztree_t *tree);

zserver_t *create_zserver(int server_id);
void destroy_zserver(zserver_t *zserver);

zclient_t *create_zclient(int client_id);
void destroy_zclient(zclient_t *zclient);
void set_zclient(zclient_t *zclient, int target, int tag);
#endif
