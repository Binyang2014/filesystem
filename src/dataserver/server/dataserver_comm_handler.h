/**
 * dataserver_communicate.h
 *
 * created on 2015.1.25
 * author: Binyang
 *
 * this file define some function used to communicate with master and client
 * following functions and structures this file should define
 * 1. send and receive message from master and client, just basic functions such do
 * read and write. About schedule things we will  leave to data server
 */

#ifndef _DATASERVER_COMM_H_
#define _DARASERVER_COMM_H_


#include "../../tool/message.h"
#include "../../global.h"
#include "../structure/vfs_structure.h"


struct msg_for_rw
{
	dataserver_file_t* file;
	off_t offset;
	size_t count;
};

typedef struct msg_for_rw msg_for_rw_t;

int m_read_handler(int source, int tag, msg_for_rw_t* file_info, char* buff, void* msg_buff);
int m_write_handler(int source, int tag, msg_for_rw_t* file_info, char* buff,  void* msg_buff);
//...other message passing functions
#endif
