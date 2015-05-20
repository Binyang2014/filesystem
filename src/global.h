/*******************************************************
  这里应当用来存放从配置文件中读取的信息
  和一些全局参数
  *****************************************************/
#ifndef _GLOBAL_H_
#define _GLOBAL_H_

//about block information
#define MAX_ALLOC_SIZE (1<<30)
#define CHUNK_SIZE  0
#define BLOCK_SIZE (1<<12)
#define N_LOG_BLOCKS_PER_G 4

//some number maybe useful
#define INF_UNSIGNED_INT (~0U)
#define INF_UNSIGNED_LONG  (~0UL)

//used by error information
#define MAXLINE 4096

//following is about message type
#define COMMON_MSG_HEAD 4
#define COMMON_MSG_LEN (4096 + COMMON_MSG_HEAD)
#define MAX_CMD_MSG_LEN 4096
#define DATA_MSG_HEAD_LEN 16
#define MAX_DATA_CONTENT_LEN 4096
#define MAX_DATA_MSG_LEN (4096 + DATA_MSG_HEAD_LEN)

//following is operation code in message
#define MSG_READ 0x00
#define MSG_WRITE 0x01
#define MSG_ACC 0x02
//...

//reserved tag when transport message
#define D_MSG_CMD_TAG 0	//command message should be sent with tag 0

//data server configure about buffer size, just a test, need to be redefined
#define D_FILE_BSIZE	(1<<8)
#define D_DATA_BSIZE	(1<<8)
#define D_PAIR_BSIZE	(1<<8)
#define D_MSG_BSIZE	(1<<8)
//线程池大小
#define D_THREAD_SIZE	(1<<4)

//use for debug
#define DEBUG 1
#define VFS_RW_DEBUG 1
#define DATASERVER_COMM_DEBUG 1
#define THREAD_POOL_DEBUG 1
#define DATASERVER_BUFF_DEBUG 1

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

/*DEFINE LOG LEVEL*/
#define LOG_DEBUG 1
#define LOG_INFO 2
#define LOG_WARN 3
#define LOG_ERROR 4
#define LOG_FATAL 5
#define LOG_LEVEL 0

extern int master_rank;
#endif
