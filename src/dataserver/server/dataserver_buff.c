/**
 * created on 2015.4.16
 * author: Binyang
 * buffer handler functions
 */
#include <string.h>
#include <stdlib.h>
#include "dataserver_buff.h"
#include "../../tool/errinfo.h"
#include "../../structure/basic_list.h"
#include "../../structure/basic_queue.h"

/*===================== Prototypes ==========================*/
static void* buff_node_dup(void* );
static void* reply_msg_dup(void* );
static void* m_data_dup(void* );
static void* file_info_dup(void* );
static void* f_arr_dup(void* );
static void* common_msg_dup(void* );

/*===================== Interface Implement ================*/
/**
 * get the list of buffer, if error return NULL
 */
list_t* get_buffer_list(data_server_t* data_server, int num)
{
	int i;
	list_t* list;
	list_node_t* node;
	basic_queue_t* buff_queue;
	if((list = list_create()) == NULL)
		return NULL;
	buff_queue = data_server->buff_node_queue;
	queue_set_dup_method(buff_queue, buff_node_dup);
	for(i = 0; i < num; i++)
	{
		buff_queue->basic_queue_op->pop(buff_queue, node);
		list->list_ops->list_add_exist_node_tail(list, node);
	}
	return list;
}

/**
 * get a block of message buffer from data server
 */
void* get_reply_msg_buffer(data_server_t* data_server)
{
	void* msg_buffer;
	basic_queue_t* buff_queue;
	buff_queue = data_server->reply_message_buff;
	queue_set_dup_method(buff_queue, reply_msg_dup);
	buff_queue->basic_queue_op->pop(buff_queue, msg_buffer);
	return msg_buffer;
}

void* get_data_buffer(data_server_t* data_server)
{
	void* data_buffer;
	basic_queue_t* buff_queue;
	buff_queue = data_server->m_data_buff;
	queue_set_dup_method(buff_queue, m_data_dup);
	buff_queue->basic_queue_op->pop(buff_queue, data_buffer);
	return data_buffer;
}

void* get_file_info(data_server_t* data_server)
{
	void* file_info_buffer;
	basic_queue_t* buff_queue;
	buff_queue = data_server->f_arr_buff;
	queue_set_dup_method(buff_queue, file_info_dup);
	buff_queue->basic_queue_op->pop(buff_queue, file_info_buffer);
	return file_info_buffer;
}

void* get_f_arr_buff(data_server_t* data_server)
{
	void* f_arr_buffer;
	basic_queue_t* buff_queue;
	buff_queue = data_server->f_arr_buff;
	queue_set_dup_method(buff_queue, f_arr_dup);
	buff_queue->basic_queue_op->pop(buff_queue, f_arr_buffer);
	return f_arr_buffer;
}

void* get_common_msg_buff(data_server_t* data_server)
{
	void* common_msg;
	basic_queue_t* buff_queue;
	buff_queue = data_server->common_msg_buff;
	queue_set_dup_method(buff_queue, common_msg_dup);
	buff_queue->basic_queue_op->pop(buff_queue, common_msg);
	return common_msg;
}

/*===================== Static Functions======================*/
/**
 * just return itself and do not allocate new room for the node
 */
static void* buff_node_dup(void* buff_node)
{
	return buff_node;
}

static void* reply_msg_dup(void* reply_msg)
{
	return reply_msg;
}

static void* m_data_dup(void* m_data)
{
	return m_data;
}
static void* file_info_dup(void* file_info)
{
	return file_info;
}

static void* f_arr_dup(void* f_arr)
{
	return f_arr;
}
static void* common_msg_dup(void* common_msg)
{
	return common_msg;
}
