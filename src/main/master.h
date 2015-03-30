/*
 * master.h
 *
 *  Created on: 2015年1月4日
 *      Author: ron
 */

#ifndef SRC_MAIN_MASTER_H_
#define SRC_MAIN_MASTER_H_
#include "opera_code_sturcture.h"
#include "namespace.h"
#include "conf.h"
#include "pthread.h"
#include "mpi.h"

/**
 * master维护的数据服务器状态
 */
struct data_server_des **data_servers;

void master_init(int rank);

void master_server();

pthread_t thread_master_log_backup, thread_master_namespace, thread_master_heart;

unsigned char buff[];

#endif /* SRC_MAIN_MASTER_H_ */
