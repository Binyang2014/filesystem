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

const int create_file_code = 100;
const int create_dir_code = 101;
const int del_file_code = 102;
const int del_fir_code = 103;
const int rename_file_code = 104;
const int renmae_dir_code = 105;

pthread_mutex_t lock_namespace;


/**
 * 创建文件，文件描述
 */
struct file_des{
	char *file_name;
	int name_length;
};

/**
 * 创建目录，目录描述
 */
struct dir_des
{
	char *dir_name;
	int name_length;
};

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

/**
 * 操作指令数据结构
 */
struct opera_des
{
	unsigned int opera_code; //操作码
	MPI_Comm mpi_comm;
};

#endif /* SRC_MAIN_OPERA_CODE_STURCTURE_H_ */
