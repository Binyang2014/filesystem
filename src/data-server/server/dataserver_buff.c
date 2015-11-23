/**
 * created on 2015.4.16
 * author: Binyang
 * buffer handler functions
 */
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "dataserver_buff.h"
#include "log.h"
#include "bitmap.h"
#include "basic_list.h"
#include "basic_queue.h"
#include "zmalloc.h"

static list_node_t* list_node_arr;
static msg_data_t* msg_data_arr;
static common_msg_t* common_msg_arr;
static dataserver_file_t* file_arr;
static vfs_hashtable_t* f_map_arr;
static vfs_hashtable_t summary_table;

/*===================== Prototypes ==========================*/
static void buff_node_dup(void*, void*);
static void m_data_dup(void*, void*);
static void file_info_dup(void*, void*);
static void f_arr_dup(void*, void*);
static void common_msg_buff_dup(void*, void*);

/*===================== Interface Implement ================*/
/**
 * get the list of buffer, if error return NULL
 */
list_t* get_buffer_list(data_server_t* data_server, int num)
{
	int i;
	list_t* list;
	list_node_t* node = NULL;
	basic_queue_t* buff_queue;
	if((list = list_create()) == NULL)
		return NULL;
	buff_queue = data_server->buff_node_queue;

	for(i = 0; i < num; i++)
	{
		buff_queue->basic_queue_op->pop(buff_queue, &node);
		list->list_ops->list_add_exist_node_tail(list, node);
	}
	return list;
}

void* get_data_buff(data_server_t* data_server)
{
	void* data_buffer = NULL;
	basic_queue_t* buff_queue;
	buff_queue = data_server->m_data_buff;

	buff_queue->basic_queue_op->pop(buff_queue, &data_buffer);
	return data_buffer;
}

void* get_file_info_buff(data_server_t* data_server)
{
	void* file_info_buffer = NULL;
	basic_queue_t* buff_queue;
	buff_queue = data_server->file_buff;

	buff_queue->basic_queue_op->pop(buff_queue, &file_info_buffer);
	return file_info_buffer;
}

void* get_common_msg_buff(data_server_t* data_server)
{
	void* common_msg = NULL;
	basic_queue_t* buff_queue;
	buff_queue = data_server->common_msg_buff;

	buff_queue->basic_queue_op->pop(buff_queue, &common_msg);
	return common_msg;
}

/*
 * get a number of buffers, because this is a array maybe I need
 * to reorganize it. How to get a series of room in memory. Maybe
 * use bitmap?
 */
void* get_f_arr_buff(data_server_t* data_server, int num)
{
	vfs_hashtable_t* f_arr_buffer = NULL;
	basic_queue_t* buff_queue;
	unsigned long pos;

	buff_queue = data_server->f_arr_buff;

	buff_queue->basic_queue_op->pop(buff_queue, &f_arr_buffer);

	f_arr_buffer->hash_table_size = num;
	pos = bitmap_find_next_zero_area(data_server->f_arr_bitmap, F_ARR_SIZE, 0, num, 0);

#ifdef DATASERVER_BUFF_DEBUG
	printf("The start position is %ld\n", pos);
#endif

	//if pos == size, do something, maybe realloc
	f_arr_buffer->blocks_arr = &summary_table.blocks_arr[pos];
	f_arr_buffer->chunks_arr = &summary_table.chunks_arr[pos];
#ifdef DATASERVER_BUFF_DEBUG
	printf("The blocks buffer address is %p\n", &summary_table.blocks_arr[pos]);
#endif
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
	buff_queue->basic_queue_op->push(buff_queue, &buff);
}

void return_data_buff(data_server_t* data_server, void* buff)
{
	basic_queue_t* buff_queue;
	buff_queue = data_server->m_data_buff;
	buff_queue->basic_queue_op->push(buff_queue, &buff);
}

void return_file_info_buff(data_server_t* data_server, void* buff)
{
	basic_queue_t* buff_queue;
	buff_queue = data_server->file_buff;
	buff_queue->basic_queue_op->push(buff_queue, &buff);
}

void return_f_arr_buff(data_server_t* data_server, vfs_hashtable_t* buff)
{
	basic_queue_t* buff_queue;
	unsigned long pos;
	int num;

	buff_queue = data_server->f_arr_buff;
	buff_queue->basic_queue_op->push(buff_queue, &buff);
	num = buff->hash_table_size;
	//Calculate start position
	pos = ((unsigned long)(buff->blocks_arr) - (unsigned long)(summary_table.blocks_arr)) /
			sizeof(uint32_t);

#ifdef DATASERVER_BUFF_DEBUG
	printf("The release position is %ld\n", pos);
#endif

	bitmap_clear(data_server->f_arr_bitmap, pos, num);
	buff->hash_table_size = 0;
	buff->blocks_arr = NULL;
	buff->chunks_arr = NULL;
}

//allocate buffer for data server. This function should be run first
void set_data_server_buff(data_server_t* data_server, int init_length)
{
	int i;
	void* ptr_temp;

	//allocate message queue
	if((data_server->buff_node_queue = alloc_basic_queue(sizeof(list_node_t*), BUFF_NODE_SIZE + 1))== NULL)
	{
		err_sys("error when allocate buffer");
	}
	if((data_server->m_data_buff= alloc_basic_queue(sizeof(msg_data_t*), init_length + 1))== NULL)
	{
		err_sys("error when allocate buffer");
	}
	if((data_server->common_msg_buff = alloc_basic_queue(sizeof(common_msg_t*), init_length + 1))== NULL)
	{
		err_sys("error when allocate buffer");
	}
	if((data_server->file_buff = alloc_basic_queue(sizeof(dataserver_file_t*), init_length + 1))== NULL)
	{
		err_sys("error when allocate buffer");
	}
	// if((data_server->reply_message_buff = alloc_basic_queue(sizeof(void*), init_length + 1))
	// 		== NULL)
	// 	err_sys("error when allocate buffer");
	if((data_server->f_arr_buff = alloc_basic_queue(sizeof(vfs_hashtable_t*), init_length + 1))
			== NULL)
		err_sys("error when allocate buffer");

	//set dup method
	queue_set_dup_method(data_server->buff_node_queue, buff_node_dup);
	queue_set_dup_method(data_server->m_data_buff, m_data_dup);
	queue_set_dup_method(data_server->common_msg_buff, common_msg_buff_dup);
	queue_set_dup_method(data_server->file_buff, file_info_dup);
	// queue_set_dup_method(data_server->reply_message_buff, reply_msg_dup);
	queue_set_dup_method(data_server->f_arr_buff, f_arr_dup);

	//allocate buffers
	if((list_node_arr = (list_node_t* )zmalloc(sizeof(list_node_t) * init_length *
			BUFF_NODE_SIZE)) == NULL)
		err_sys("error when allocate buffer");
	if((msg_data_arr = (msg_data_t* )zmalloc(sizeof(msg_data_t) * init_length)) == NULL)
		err_sys("error when allocate buffer");
	if((common_msg_arr = (common_msg_t* )zmalloc(sizeof(common_msg_t) * init_length)) == NULL)
		err_sys("error when allocate buffer");
	if((file_arr = (dataserver_file_t* )zmalloc(sizeof(dataserver_file_t) * init_length))
			== NULL)
		err_sys("error when allocate buffer");
	// if((reply_message_arr = (void* )zmalloc(MAX_CMD_MSG_LEN * init_length)) == NULL)
	// 	err_sys("error when allocate buffer");
	if((f_map_arr = (vfs_hashtable_t* )zmalloc(sizeof(vfs_hashtable_t) * F_ARR_SIZE)) == NULL)
		err_sys("error when allocate buffer");

	//allocate for data server array map structure
	summary_table.hash_table_size = F_ARR_SIZE;
	if((summary_table.blocks_arr = (uint32_t* )zmalloc(sizeof(uint32_t)	* F_ARR_SIZE))== NULL)
	{
		err_sys("error when allocate buffer");
	}
	if((summary_table.chunks_arr = (uint64_t* )zmalloc(sizeof(uint64_t) * F_ARR_SIZE)) == NULL)
	{
		err_sys("error when allocate buffer");
	}
	if((data_server->f_arr_bitmap = (unsigned long*)zmalloc(sizeof(unsigned long) * F_ARR_SIZE / BITS_PER_LONG)) == NULL)
	{
		err_sys("error when allocate buffer");
	}

	//initial data server buffer by set value
	for(i = 0; i < init_length; i++)
	{
		ptr_temp = &msg_data_arr[i];
		data_server->m_data_buff->basic_queue_op->push(data_server->m_data_buff, &ptr_temp);

		ptr_temp = &common_msg_arr[i];
		data_server->common_msg_buff->basic_queue_op->push(data_server->common_msg_buff, &ptr_temp);

		ptr_temp = &file_arr[i];
		data_server->file_buff->basic_queue_op->push(data_server->file_buff, &ptr_temp);

		ptr_temp = &f_map_arr[i];
		data_server->f_arr_buff->basic_queue_op->push(data_server->f_arr_buff, &ptr_temp);
	}
	for(i = 0; i < BUFF_NODE_SIZE; i++)
	{
		ptr_temp = &list_node_arr[i];
		list_node_arr[i].value = NULL;
		list_node_arr[i].next = NULL;
		list_node_arr[i].prev = NULL;
		data_server->buff_node_queue->basic_queue_op->push(data_server->buff_node_queue, &ptr_temp);
	}
}

void free_data_server_buff(data_server_t *data_server)
{
	//free map structure
	zfree(data_server->f_arr_bitmap);
	zfree(summary_table.blocks_arr);
	zfree(summary_table.chunks_arr);

	zfree(f_map_arr);
	zfree(file_arr);
	zfree(common_msg_arr);
	zfree(msg_data_arr);
	zfree(list_node_arr);

	//free message queue
	destroy_basic_queue(data_server->f_arr_buff);
	destroy_basic_queue(data_server->file_buff);
	destroy_basic_queue(data_server->common_msg_buff);
	destroy_basic_queue(data_server->m_data_buff);
	destroy_basic_queue(data_server->buff_node_queue);
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
		buff_queue->basic_queue_op->push(buff_queue, &node);
	}
	list->list_ops->list_release_iterator(iter);
	list_release_without_node(list);
}

/*===================== Static Functions======================*/
/**
 * just return itself and do not allocate new room for the node
 */
static void buff_node_dup(void* dest, void* buff_node)
{
	*(void** )dest = *(void** )buff_node;
}

static void m_data_dup(void* dest, void* m_data)
{
	*(void** )dest = *(void** )m_data;
}
static void file_info_dup(void* dest, void* file_info)
{
	*(void** )dest = *(void** )file_info;
}

static void f_arr_dup(void* dest, void* f_arr)
{
	*(void** )dest = *(void** )f_arr;
}

static void common_msg_buff_dup(void* dest, void* common_msg)
{
	*(void** )dest = *(void** )common_msg;
}
