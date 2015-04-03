/*
 * opera_code_sturcture.h
 * 操作指令的数据结构
 *  Created on: 2015年1月1日
 *      Author: ron
 */
#ifndef SRC_MAIN_OPERA_CODE_STURCTURE_H_
#define SRC_MAIN_OPERA_CODE_STURCTURE_H_
#include "mpi.h"
#include "pthread.h"

pthread_mutex_t lock_namespace;


/**
 * 数据服务器描述
 */
struct data_server_des
{
	int id;				//机器id
	long last_time;		//上次交互时间
	int total_memory;  	//可用的总内存
	int used_memeory;	//使用的内存
	int avail_memory;	//可用的内存
};

#endif /* SRC_MAIN_OPERA_CODE_STURCTURE_H_ */
