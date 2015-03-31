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
#include "../tool/message.h"

/**
 * master维护的数据服务器状态
 */
struct data_server_des **data_servers;


static void master_init(int rank);

static void master_server();

static void log_backup();

static void heart_blood();

static void namespace_control();

/**
 *
 */

pthread_t thread_master_log_backup, thread_master_namespace, thread_master_heart;

unsigned char buff[100];



#endif /* SRC_MAIN_MASTER_H_ */
