/*
 * rpc_server.h
 *
 *  Created on: 2015年7月10日
 *      Author: ron
 */

#ifndef SRC_COMMON_COMMUNICATION_RPC_SERVER_H_
#define SRC_COMMON_COMMUNICATION_RPC_SERVER_H_

#include <stdint.h>
#include <pthread.h>
#include "message.h"
#include "threadpool.h"
#include "global.h"

struct rpc_send_msg {
	int dst;
	int tag;
	int length;
	void* msg;
};

struct rpc_server {
	int server_id;
	int server_thread_cancel;
	int server_commit_cancel;//let server to stop
	void *recv_buff;
	struct rpc_send_msg *send_buff;//this buffer just for queue
	struct rpc_server_op *op;
	syn_queue_t *request_queue;
	syn_queue_t *send_queue;
	thread_pool_t *thread_pool;
	pthread_t qsend_tid;
};

struct rpc_server_op {
	void (*server_start)(struct rpc_server *server);
	void (*server_start2)(struct rpc_server *server);
	void (*server_stop)(struct rpc_server *server);
	void (*server_stop2)(struct rpc_server *server);
	int (*send_result)(void *param, int source, int tag, int len,
			msg_type_t type);
	void (*send_to_queue)(struct rpc_server *server, void* param, int dst, int tag,
			int len);
	int (*recv_reply)(void* param, int source, int tag, msg_type_t type);
};

typedef struct rpc_send_msg rpc_send_msg_t;
typedef struct rpc_server rpc_server_t;
typedef struct rpc_server_op rpc_server_op_t;

rpc_server_t *create_rpc_server(int thread_num, int queue_size, int server_id, 
		resolve_handler_t resolve_handler);
//create rpc server with send queue
rpc_server_t *create_rpc_server2(int thread_num, int recv_qsize, int send_qsize, 
		int server_id, resolve_handler_t resolve_handler);
void force_destroy_rpc_server(rpc_server_t *server);
void destroy_rpc_server(rpc_server_t *server);

int init_server_stop_handler(event_handler_t *, void* server, void*
		common_msg);
void server_stop_handler(event_handler_t *event_handler);

#endif /* SRC_COMMON_COMMUNICATION_RPC_SERVER_H_ */
