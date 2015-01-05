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
}

void master_server(){

}

void log_backup(){

}

void heart_blood(){

}

void namespace_control(){
	while(1){

	}
}


