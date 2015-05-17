/**
 * created on 2015.4.16
 * author: Binyang
 * buffer handler functions
 */
#include <string.h>
#include <stdlib.h>
#include "dataserver_buff.h"
#include "dataserver.h"
#include "../../tool/errinfo.h"
#include "../../structure/bitmap.h"
#include "../../structure/basic_list.h"
#include "../../structure/basic_queue.h"

/*===================== Prototypes ==========================*/
static void* buff_node_dup(void*, void*);
static void* reply_msg_dup(void*, void*);
static void* m_data_dup(void*, void*);
static void* file_info_dup(void*, void*);
static void* f_arr_dup(void*, void*);
static void* common_msg_dup(void*, void*);

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
void* get_reply_msg_buff(data_server_t* data_server)
{
	void* msg_buffer;
	basic_queue_t* buff_queue;
	buff_queue = data_server->reply_message_buff;

	buff_queue->basic_queue_op->pop(buff_queue, msg_buffer);
	return msg_buffer;
}

void* get_data_buff(data_server_t* data_server)
{
	void* data_buffer;
	basic_queue_t* buff_queue;
	buff_queue = data_server->m_data_buff;

	buff_queue->basic_queue_op->pop(buff_queue, data_buffer);
	return data_buffer;
}

void* get_file_info_buff(data_server_t* data_server)
{
	void* file_info_buffer;
	basic_queue_t* buff_queue;
	buff_queue = data_server->f_arr_buff;

	buff_queue->basic_queue_op->pop(buff_queue, file_info_buffer);
	return file_info_buffer;
}

void* get_common_msg_buff(data_server_t* data_server)
{
	void* common_msg;
	basic_queue_t* buff_queue;
	buff_queue = data_server->common_msg_buff;

	buff_queue->basic_queue_op->pop(buff_queue, common_msg);
	return common_msg;
}

/*
 * get the number of buffers, because this is a array maybe I need
 * to reorganize it. How to get a series of room in memory. Maybe
 * use bitmap?
 */
void* get_f_arr_buff(data_server_t* data_server, int num)
{
	vfs_hashtable_t* f_arr_buffer;
	basic_queue_t* buff_queue;
	unsigned long pos;

	buff_queue = data_server->f_arr_buff;

	buff_queue->basic_queue_op->pop(buff_queue, f_arr_buffer);

	f_arr_buffer->hash_table_size = num;
	pos = bitmap_find_next_zero_area(data_server->f_arr_bitmap, F_ARR_SIZE, 0, num, 0);
	//if pos == size, do something, maybe realloc
	f_arr_buffer->blocks_arr = &data_server->blocks_arr_buff[pos];
	f_arr_buffer->chunks_arr = &data_server->chunks_arr_buff[pos];
	bitmap_set(data_server->f_arr_bitmap, pos, num);
	return f_arr_buffer;
}

/**
 * return a block of message buffer to data server
 */
void reture_common_msg_buff(data_server_t* data_server, void* buff)
{
	basic_queue_t* buff_queue;
	buff_queue = data_server->common_msg_buff;
	buff_queue->basic_queue_op->push(data_server->common_msg_buff, buff);
}

void return_reply_msg_buff(data_server_t* data_server, void* buff)
{
	basic_queue_t* buff_queue;
	buff_queue = data_server->reply_message_buff;
	buff_queue->basic_queue_op->push(data_server->common_msg_buff, buff);
}

void return_data_buff(data_server_t* data_server, void* buff)
{
	basic_queue_t* buff_queue;
	buff_queue = data_server->m_data_buff;
	buff_queue->basic_queue_op->push(data_server->common_msg_buff, buff);
}

void return_file_info_buff(data_server_t* data_server, void* buff)
{
	basic_queue_t* buff_queue;
	buff_queue = data_server->file_buff;
	buff_queue->basic_queue_op->push(data_server->common_msg_buff, buff);
}

void return_f_arr_buff(data_server_t* data_server, vfs_hashtable_t* buff)
{
	basic_queue_t* buff_queue;
	unsigned long pos;
	int num;

	buff_queue = data_server->f_arr_buff;
	buff_queue->basic_queue_op->push(buff_queue, buff);
	num = buff->hash_table_size;
	//Calculate start position
	pos = (buff->blocks_arr - data_server->blocks_arr_buff) / sizeof(unsigned int);

	bitmap_clear(data_server->f_arr_bitmap, pos, num);
}

//allocate buffer for data server. This function should be run first
void set_data_server_buff(data_server_t* data_server, int init_length)
{
	//allocate message queue
	if((data_server->buff_node_queue = alloc_msg_queue(sizeof(list_node_t), init_length))
			== NULL)
		err_sys("error when allocate buffer");
	if((data_server->m_data_buff= alloc_msg_queue(sizeof(msg_data_t), init_length))
			== NULL)
		err_sys("error when allocate buffer");
	if((data_server->common_msg_buff = alloc_msg_queue(sizeof(common_msg_t), init_length))
			== NULL)
		err_sys("error when allocate buffer");
	if((data_server->file_buff = alloc_msg_queue(sizeof(dataserver_file_t), init_length))
			== NULL)
		err_sys("error when allocate buffer");
	if((data_server->reply_message_buff = alloc_msg_queue(MAX_CMD_MSG_LEN, init_length))
			== NULL)
		err_sys("error when allocate buffer");
	if((data_server->f_arr_buff = alloc_msg_queue(sizeof(vfs_hashtable_t), init_length))
			== NULL)
		err_sys("error when allocate buffer");

	//set dup method
	queue_set_dup_method(data_server->buff_node_queue, buff_node_dup);
	queue_set_dup_method(data_server->m_data_buff, m_data_dup);
	queue_set_dup_method(data_server->common_msg_buff, common_msg_dup);
	queue_set_dup_method(data_server->file_buff, file_info_dup);
	queue_set_dup_method(data_server->reply_message_buff, reply_msg_dup);
	queue_set_dup_method(data_server->f_arr_buff, f_arr_dup);

	if((data_server->blocks_arr_buff = (unsigned int* )molloc(sizeof(unsigned int)
			* F_ARR_SIZE)) == NULL)
		err_sys("error when allocate buffer");
	if((data_server->chunks_arr_buff = (unsigned long long* )molloc(sizeof(unsigned long long)
			* F_ARR_SIZE)) == NULL)
		err_sys("error when allocate buffer");
	if((data_server->f_arr_bitmap = (unsigned long*)molloc(sizeof(unsigned long) *
			F_ARR_SIZE / BITS_PER_LONG)) == NULL)
		err_sys("error when allocate buffer");
}

void return_buffer_list(data_server_t* data_server, list_t* list)
{
	int i, num;
	list_node_t* node;
	basic_queue_t* buff_queue;
	list_iter_t *iter;

	num = list->len;
	buff_queue = data_server->buff_node_queue;

	iter = list->list_ops->list_get_iterator(list, AL_START_HEAD);
	for(i = 0; i < num; i++)
	{
		node = list->list_ops->list_next(iter);
		buff_queue->basic_queue_op->push(buff_queue, node);
	}
	list_release_without_node(list);
}

/*===================== Static Functions======================*/
/**
 * just return itself and do not allocate new room for the node
 */
static void buff_node_dup(void* dest, void* buff_node)
{
	(void* )*dest = buff_node;
}

static void reply_msg_dup(void* dest, void* reply_msg)
{
	(void* )*dest =  reply_msg;
}

static void m_data_dup(void* dest, void* m_data)
{
	(void* )*dest = m_data;
}
static void file_info_dup(void* dest, void* file_info)
{
	(void* )*dest = file_info;
}

static void f_arr_dup(void* dest, void* f_arr)
{
	(void* )*dest = f_arr;
}

static void common_msg_dup(void* dest, void* common_msg)
{
	(void* )*dest = common_msg;
}
