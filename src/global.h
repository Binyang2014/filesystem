/*******************************************************
  这里应当用来存放从配置文件中读取的信息
  和一些全局参数
  *****************************************************/
#ifndef _GLOBAL_H_
#define _GLOBAL_H_
#define MAX_ALLOC_SIZE (1<<30)
#define CHUNK_SIZE  0
#define INF_UNSIGNED_INT (~0U)
#define INF_UNSIGNED_LONG  (~0UL)

#define MAXLINE 4096

//following is about message type
#define MAX_COM_MSG_LEN 4096

//following is operation code in message
#define MSG_READ 0x00
#define MSG_WRITE 0x01
//...

//use for debug
#define DEBUG 1
#define VFS_RW_DEBUG 1
#endif
