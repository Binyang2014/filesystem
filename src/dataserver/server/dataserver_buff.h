/**
 * created on 2015.4.16
 * author: Binyang
 * This file will offer many functions to handle the data server's
 * buffer. dataserver.h must include this file
 */
#ifndef _DATASERVER_BUFF_H_
#define _DATASERVER_BUFF_H_

#include <stdio.h>
#include "../../tool/message.h"
#include "../../global.h"
#include "../../structure/basic_list.h"

//functions about how to handle buffer in data server
list_t* get_buffer_list(data_server_t*, int);
void* get_common_msg_buff(data_server_t*);
void* get_reply_msg_buff(data_server_t*);
void* get_data_buff(data_server_t*);
void* get_file_info_buff(data_server_t*);
void* get_f_arr_buff(data_server_t*, int num);

void reture_common_msg_buff(data_server_t*, void** );
void return_reply_msg_buff(data_server_t*, void*);
void return_data_buff(data_server_t*, void*);
void return_file_info_buff(data_server_t*, void*);
void return_f_arr_buff(data_server_t*, vfs_hashtable_t*);

void set_data_server_buff(data_server_t*, int);
void return_buffer_list(data_server_t*);
#endif
//void
