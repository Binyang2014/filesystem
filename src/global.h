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

//DEBUG
#define GLOBAL_DEBUG 1
#define DEBUG 1
#define RPC_SERVER_DEBUG 0
#define MPI_COMMUNICATION_DEBUG 0
#define VFS_RW_DEBUG 1
#define DATASERVER_COMM_DEBUG 1
#define THREAD_POOL_DEBUG 0
#define DATASERVER_BUFF_DEBUG 1
#define MAP_DEBUG 0
#define MACHINE_ROLE_DEBUG 1


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
