/*******************************************************
  这里应当用来存放从配置文件中读取的信息
  和一些全局参数
  *****************************************************/
#ifndef _GLOBAL_H_
#define _GLOBAL_H_

//about block information
#define MAX_ALLOC_SIZE (1<<30)
#define CHUNK_SIZE  0
#define BLOCK_SIZE (1<<20)
#define N_LOG_BLOCKS_PER_G 4

//some number maybe useful
#define INF_UNSIGNED_INT (~0U)
#define INF_UNSIGNED_LONG  (~0UL)

//used by error information
#define MAXLINE 4096

//following is about message length, and only fixed lenght can be defined here
#define MAX_CMD_MSG_LEN 1024
#define MAX_DATA_CONTENT_LEN (1<<22)

//data server configure about buffer size, just a test, need to be redefined
#define D_FILE_BSIZE	(1<<8)
#define D_DATA_BSIZE	(1<<8)
#define D_PAIR_BSIZE	(1<<8)
#define D_MSG_BSIZE	(1<<8)
//线程池大小
#define D_THREAD_SIZE	(1<<4)

#define DEBUG 0
#define DEBUG_Y DEBUG & 1
#define DEBUG_N DEBUG & 1
//DEBUG
#define CLIENT_DEBUG DEBUG_Y
#define RPC_SERVER_DEBUG DEBUG_Y
#define RPC_CLIENT_DEBUG DEBUG_Y
#define MPI_COMMUNICATION_DEBUG DEBUG_Y
#define VFS_RW_DEBUG DEBUG_N
#define DATASERVER_COMM_DEBUG DEBUG_N
#define THREAD_POOL_DEBUG DEBUG_Y
#define DATASERVER_BUFF_DEBUG DEBUG_N
#define MAP_DEBUG DEBUG_N
#define MACHINE_ROLE_DEBUG DEBUG_N
#define MACHINE_ROLE_FETCHER_DEBUG DEBUG_N
#define DATA_MASTER_DEBUG DEBUG_Y
#define ZSERVER_DEBUG DEBUG_Y
#define ZCLIENT_DEBUG DEBUG_Y

#define DATA_MASTER_PRINT 1

//define for file access mode
#define RUSR 0400
#define WUSR 0200
#define XUSR 0100
#define RGRP 0040
#define WGRP 0020
#define XGRP 0010
#define ROTH 0004
#define WOTH 0002
#define XOTH 0001

//define file open mode
#define RDONLY 0001
#define WRONLY 0002
#define RDWR 0004
#define APPEND 0010
#define CREATE_TEMP 0020
#define CREATE_PERSIST 0040

//define optional size of file system
enum TOTAL_SIZE
{
	SMALLEST = 24,	//16MB
	SMALL = 26,		//64MB
	MIDDLE = 28,	//256MB
	LARGE = 30,		//1GB
	LARGEST = 32	//4GB
};

typedef enum TOTAL_SIZE total_size_t;
typedef unsigned short f_mode_t;
typedef unsigned short open_mode_t;

#endif
