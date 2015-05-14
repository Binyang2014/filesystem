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
void* get_common_msg_buff(data_server_t*,);
void* get_reply_msg_buffer(data_server_t*);
void* get_data_buffer(data_server_t*);
void* get_file_info(data_server_t*);
void* get_f_arr_buff(data_server_t*);

#endif
//void
