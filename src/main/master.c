/*
 * master.c
 *
 *  Created on: 2015年1月4日
 *      Author: ron
 */
#include "master.h"

void master_init(int rank)
{
	master->rank = rank;
	master->comm = MPI_COMM_WORLD;
	pthread_create(&thread_master_namespace, NULL, namespace_control(), NULL);
	pthread_create(&thread_master_log_backup, NULL, log_backup(), NULL);
}

/**
 *	master的服务
 */
void master_server(){

}

/**
 * master记录日志
 */
void log_backup(){

}

/*
 * master接受心跳信息
 */
void heart_blood(){

}

/**
 *	命名空间的操作
 */
void namespace_control(){
	while(1){

	}
}


