/*
 * clinet.h
 *
 *  Created on: 2015年1月4日
 *      Author: ron
 */

#ifndef SRC_MAIN_CLIENT_H_
#define SRC_MAIN_CLIENT_H_
#include "master.h"
#include "opera_code_sturcture.h"

enum client_request{
	create_file,
	create_dir,
	del_file,
	del_dir,
	rename_file,
	rename_dir,
	mv_file,
	mv_dir
};
/**
 * 请求事件队列节点
 */
struct message_queue{
	int queue_size;
	struct request_queue{
		enum client_request request_type;
		void *message_type;
		struct request_queue *next_request;
	};
};

void client_server();

#endif /* SRC_MAIN_CLIENT_H_ */
