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
#include "../../structure/buffer.h"

//functions about how to handle buffer in data server
buffer_t* get_series_buffer(data_server_t*, int);
buffer_t* get_common_msg_buff(data_server_t*, common_msg_t*);
buffer_t* get_msg_buffer(data_server_t*);
buffer_t* get_data_buffer(data_server_t*);
buffer_t* get_file_info(data_server_t*);
buffer_t* get_f_arr_buff(data_server_t*);

#endif
//void
