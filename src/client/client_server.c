/*
 * create on: 2015.10.19
 * author: Binyang
 * This file will implement functions define in clinet_server.h
 */
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include "log.h"
#include "client_server.h"
#include "zmalloc.h"
#include "message.h"

static int get_fd(fclient_t *fclient);
static void init_create_msg(client_create_file_t *, const createfile_msg_t *);
static void init_append_msg(client_append_file_t *, const appendfile_msg_t *,
		const opened_file_t *);
static void init_read_msg(client_read_file_t *, const readfile_msg_t *, const
		opened_file_t *);
static void init_remove_msg(client_remove_file_t *, const removefile_msg_t *);
//static void init_file_struct(opened_file_t *opened_file, const file_ret_t *file_ret);
static int code_transfer(uint32_t op_status);

static int add_write_lock(zclient_t *zclient, const char *file_path,
		pthread_mutex_t *mutex, sds lock_name);
static int add_read_lock(zclient_t *zclient, const char *file_path,
		pthread_mutex_t *mutex, sds lock_name);
static void *watch_handler_delete(void *args);


static int append_data(fclient_t *fclient, appendfile_msg_t *writefile_msg,
		void *file_ret);
static int read_data(fclient_t *fclient, readfile_msg_t *readfile_msg,
		void *file_ret);

static void f_create(fclient_t *fclient, createfile_msg_t *createfile_msg);
static void f_open(fclient_t *fclient, openfile_msg_t *openfile_msg);
static void f_append(fclient_t *fclient, appendfile_msg_t *writefile_msg);
static void f_read(fclient_t *fclient, readfile_msg_t *readfile_msg);
static void f_close(fclient_t *fclient, closefile_msg_t *closefile_msg);
static void f_remove(fclient_t *fclient, removefile_msg_t *removefile_msg);

//=======================UTILITY FUNCTIONS======================
static int code_transfer(uint32_t op_status)
{
	switch(op_status)
	{
		case FILE_NOT_EXIST:
			return FNO_EXISIT;
		default:
			return FOK;
	}
	return FOK;
}

static int get_fd(fclient_t *fclient)
{
	int fd;

	fd = find_first_zero_bit(fclient->bitmap, MAX_FD_NUMBER);
	bitmap_set(fclient->bitmap, fd, 1);
	return fd;
}

static void init_create_msg(client_create_file_t
		*client_create_file, const createfile_msg_t *createfile_msg)
{
	if(createfile_msg->open_mode & CREATE_TEMP)
	{
		client_create_file->operation_code = CREATE_TEMP_FILE_CODE;
	}
	else if(createfile_msg->open_mode & CREATE_PERSIST)
	{
		client_create_file->operation_code = CREATE_PERSIST_FILE_CODE;
	}
	client_create_file->file_mode = createfile_msg->mode;
	strcpy(client_create_file->file_name, createfile_msg->file_path);
}

static void init_append_msg(client_append_file_t *client_append_file, const
		appendfile_msg_t *appendfile_msg, const opened_file_t *opened_file)
{
	client_append_file->operation_code = APPEND_FILE_CODE;
	strcpy(client_append_file->file_name, opened_file->f_info.file_path);
	client_append_file->write_size = appendfile_msg->data_len;
}

static void init_read_msg(client_read_file_t * client_read_file, const
		readfile_msg_t * readfile_msg, const opened_file_t *opened_file)
{
	client_read_file->operation_code = READ_FILE_CODE;
	strcpy(client_read_file->file_name, opened_file->f_info.file_path);
	client_read_file->read_size = readfile_msg->data_len;
	client_read_file->offset = opened_file->f_info.file_offset;
}

static void init_remove_msg(client_remove_file_t *client_remove_file, const
		removefile_msg_t *removefile_msg)
{
	client_remove_file->operation_code = DELETE_TMP_FILE_CODE;
	strcpy(client_remove_file->file_name, removefile_msg->file_path);
}

//We don't need this function for now, may be I will use it later.
//static void init_file_struct(opened_file_t *opened_file, const file_ret_t *file_ret)
//{
//	int dataserver_num, chunks_num, index = 0;
//	int i, j;
//	list_t *data_nodes = opened_file->f_info.data_nodes;
//	data_node_t *data_node;
//
//	dataserver_num = file_ret->dataserver_num;
//	chunks_num = file_ret->chunks_num;
//	opened_file->f_info.file_len = file_ret->file_size;
//	opened_file->f_info.file_offset = file_ret->offset;
//	for(i = 0; i < dataserver_num; i++)
//	{
//		data_node = zmalloc(sizeof(data_node_t));
//		data_node->data_server_id = file_ret->data_server_arr[i];
//		data_node->chunks_id = zmalloc(sizeof(uint64_t) * chunks_num);
//		for(j = 0; j < file_ret->data_server_cnum[i]; j++)
//			data_node->chunks_id[j] = file_ret->chunks_id_arr[index++];
//		data_nodes->list_ops->list_add_node_tail(data_nodes, data_node);
//	}
//}

static int add_write_lock(zclient_t *zclient, const char *file_path, pthread_mutex_t *mutex, sds lock_name)
{
	sds path, data, return_data, return_name;
	sds *children;
	int ret_num, count, i;
	char *begin;

	path = sds_new(file_path);
	path = sds_cat(path, ":lock/write-");
	data = sds_new("This is a write lock");
	return_data = sds_new_len(NULL, MAX_RET_DATA_LEN);
	return_name = sds_new_len(NULL, MAX_RET_DATA_LEN);
	ret_num = zclient->op->create_parent(zclient, path, data, EPHEMERAL_SQUENTIAL, return_name);
	//copy lock name
	lock_name = sds_cpy(lock_name, return_name);
	path = sds_cpy(path, file_path);
	path = sds_cat(path, ":lock");
	ret_num = zclient->op->get_children(zclient, path, return_data);
	children = sds_split_len(return_data, strlen(return_data), " ", 1, &count);
	//no more other lock
	if(count == 1)
	{
		pthread_mutex_unlock(mutex);
	}
	//there are other locks
	else
	{
		begin = return_name + strlen(return_name);
		while(*begin != '/')
		{
			begin--;
		}
		begin = begin + 1;

		for(i = 0; i < count; i++)
		{
			if(strcpy(children[i], begin) == 0)
			{
				break;
			}
		}
		path = sds_cpy(path, file_path);
		path = sds_cat(path, ":lock/");
		path = sds_cat(path, children[i - 1]);
		zclient->op->exists_znode(zclient, path, NULL, NOTICE_DELETE, watch_handler_delete, mutex);
	}

	sds_free(path);
	sds_free(data);
	sds_free(return_data);
	sds_free(return_name);
	sds_free_split_res(children, count);

	return ret_num;
}

static int add_read_lock(zclient_t *zclient, const char *file_path,
		pthread_mutex_t *mutex, sds lock_name)
{
	sds path, data, return_data, return_name;
	sds *children;
	int ret_num, count, i, j;
	char *begin;

	path = sds_new(file_path);
	path = sds_cat(path, ":lock/read-");
	data = sds_new("This is a read lock");
	return_data = sds_new_len(NULL, MAX_RET_DATA_LEN);
	return_name = sds_new_len(NULL, MAX_RET_DATA_LEN);
	ret_num = zclient->op->create_parent(zclient, path, data, EPHEMERAL_SQUENTIAL, return_name);
	//copy lock name
	lock_name = sds_cpy(lock_name, return_name);
	path = sds_cpy(path, file_path);
	path = sds_cat(path, ":lock");
	ret_num = zclient->op->get_children(zclient, path, return_data);
	children = sds_split_len(return_data, strlen(return_data), " ", 1, &count);
	//no more other lock
	if(count == 1)
	{
		pthread_mutex_unlock(mutex);
	}
	//there are other locks
	else
	{
		begin = return_name + strlen(return_name);
		while(*begin != '/')
		{
			begin--;
		}
		begin = begin + 1;

		for(i = 0; i < count; i++)
		{
			if(strcpy(children[i], begin) == 0)
			{
				break;
			}
		}
		for(j = i - 1; j > 0; j--)
		{
			if(children[j][0] == 'w')
			{
				break;
			}
		}
		if(j < 0)
		{
			pthread_mutex_unlock(mutex);
		}
		else
		{
			path = sds_cpy(path, file_path);
			path = sds_cat(path, ":lock/");
			path = sds_cat(path, children[j]);
			zclient->op->exists_znode(zclient, path, NULL, NOTICE_DELETE, watch_handler_delete, mutex);
		}
	}

	sds_free(path);
	sds_free(data);
	sds_free(return_data);
	sds_free(return_name);
	sds_free_split_res(children, count);

	return ret_num;
}
static void *watch_handler_delete(void *args)
{
	pthread_mutex_t *mutex;

	mutex = args;
	pthread_mutex_unlock(mutex);

	return NULL;
}

static int append_data(fclient_t *fclient, appendfile_msg_t *appendfile_msg, void *pos)
{
	int dataserver_num;
	int i, j, ret;
	rpc_client_t *rpc_client;
	write_c_to_d_t write_msg;
	char *data_msg;
	uint64_t *head;
	file_ret_t *file_ret;
	position_des_t *position;

	head = (uint64_t *)pos;
	file_ret = (file_ret_t *)(head + 1);//TODO not hard code here
	position = (position_des_t *)(head + 4);
	dataserver_num = file_ret->dataserver_num;
	rpc_client = fclient->rpc_client;
	write_msg.operation_code = C_D_WRITE_BLOCK_CODE;
	write_msg.unique_tag = rpc_client->tag;
	write_msg.offset = file_ret->offset;
	for(i = 0; i < dataserver_num; i++)
	{
		position_des_t *posi = position + i;
		int server_id = posi->rank;

		rpc_client->target = server_id;
		write_msg.chunks_count = posi->end - posi->start + 1;
		write_msg.write_len = BLOCK_SIZE * (write_msg.chunks_count) - write_msg.offset;
		data_msg = zmalloc(write_msg.write_len);
		read(fclient->fifo_rfd, data_msg, write_msg.write_len);

		for(j = posi->start; j <= posi->end; j++)
		{
			write_msg.chunks_id_arr[j - posi->start] = j;
		}

		rpc_client->op->set_send_buff(rpc_client, &write_msg, sizeof(write_c_to_d_t));
		rpc_client->op->set_second_send_buff(rpc_client, data_msg, write_msg.write_len);
		ret = rpc_client->op->execute(rpc_client, WRITE_C_TO_D);
		if(ret < 0)
		{
			zfree(data_msg);
			rpc_client->target = fclient->data_master_id;
			return -1;
		}
		write_msg.offset = 0;
		zfree(data_msg);
	}
	rpc_client->target = fclient->data_master_id;

	return 0;
}

static int read_data(fclient_t *fclient, readfile_msg_t *readfile_msg, void *pos)
{
	int dataserver_num;
	int i, j, ret, read_times;
	rpc_client_t *rpc_client;
	read_c_to_d_t read_msg;
	char *data_msg;
	uint64_t *head;
	file_ret_t *file_ret;
	position_des_t *position;

	head = (uint64_t *)pos;
	file_ret = (file_ret_t *)(head + 1);
	position = (position_des_t *)(head + 4);
	dataserver_num = file_ret->dataserver_num;
	rpc_client = fclient->rpc_client;
	read_msg.operation_code = C_D_READ_BLOCK_CODE;
	read_msg.unique_tag = rpc_client->tag;
	//send read times to client
	read_times = dataserver_num;
	read_msg.offset = file_ret->offset;
	write(fclient->fifo_wfd, &read_times, sizeof(int));
	//read data and send data to client
	for(i = 0; i < dataserver_num; i++)
	{
		position_des_t *posi = position + i;
		int server_id = posi->rank;

		rpc_client->target = server_id;

		read_msg.chunks_count = posi->end - posi->start + 1;
		read_msg.read_len = read_msg.chunks_count * BLOCK_SIZE - read_msg.offset;
		data_msg = zmalloc(read_msg.read_len);


		for(j = posi->start; j <= posi->end; j++)
		{
			read_msg.chunks_id_arr[j - posi->start] = j;
		}

		rpc_client->op->set_send_buff(rpc_client, &read_msg, sizeof(read_c_to_d_t));
		rpc_client->op->set_recv_buff(rpc_client, data_msg, read_msg.read_len);
		ret = rpc_client->op->execute(rpc_client, READ_C_TO_D);
		write(fclient->fifo_wfd, data_msg, read_msg.read_len);
		if(ret < 0)
		{
			zfree(data_msg);
			rpc_client->op->set_recv_buff(rpc_client, file_ret, sizeof(file_ret));
			rpc_client->target = fclient->data_master_id;
			return -1;
		}
		read_msg.offset = 0;
		zfree(data_msg);
	}
	rpc_client->target = fclient->data_master_id;
	rpc_client->op->set_recv_buff(rpc_client, file_ret, sizeof(file_ret));

	return 0;
}
//===============================ACTUAL OPERATIONS=============================
//this function is going to finish file create operation, there will be message
//passing between client and server, after it will add a open file structure to
//opened file list.
static void f_create(fclient_t *fclient, createfile_msg_t *createfile_msg)
{
	rpc_client_t *rpc_client = NULL;
	client_create_file_t *client_create_file = NULL;
	int fifo_wfd;
	opened_file_t *opened_file = NULL;
	list_t *file_list = NULL;
	zclient_t *zclient = NULL;
	sds path, data;
	file_ret_msg_t ret_msg;

	log_write(LOG_DEBUG, "creating a file to store data");
	//construct client_create_file message
	rpc_client = fclient->rpc_client;
	client_create_file = zmalloc(sizeof(client_create_file_t));
	init_create_msg(client_create_file, createfile_msg);
	client_create_file->unique_tag = rpc_client->tag;

	//send create message to data master
	rpc_client->op->set_send_buff(rpc_client, client_create_file, sizeof(client_create_file_t));
	if(rpc_client->op->execute(rpc_client, COMMAND_WITH_RETURN) < 0)
	{
		ret_msg.ret_code = FSERVER_ERR;
	}

	//create znode on server
	path = sds_new(createfile_msg->file_path);
	//have not used data structure now
	data = sds_new("msg:");
	zclient = fclient->zclient;
	if(zclient->op->create_parent(zclient, path, data, PERSISTENT, NULL) != ZOK)
	{
		ret_msg.ret_code = FSERVER_ERR;
	}
	//create lock for this file
	path = sds_cat(path, ":lock");
	data = sds_cpy(data, "This is a lock");
	zclient->op->create_parent(zclient, path, "", PERSISTENT, NULL);
	sds_free(path);
	sds_free(data);

	//create opened_file structure and add it to list
	file_list = fclient->file_list;
	opened_file = create_openedfile(createfile_msg->file_path, 0, createfile_msg->open_mode);
	opened_file->fd = get_fd(fclient);
	opened_file->f_info.file_len = 0;

	file_list->list_ops->list_add_node_tail(file_list, opened_file);

	//copy result to fd
	fifo_wfd = fclient->fifo_wfd;
	ret_msg.fd = opened_file->fd;
	if(ret_msg.ret_code != FSERVER_ERR)
	{
		ret_msg.ret_code = code_transfer(((file_sim_ret_t*)rpc_client->recv_buff)->op_status);
	}
	write(fifo_wfd, &ret_msg, sizeof(file_ret_msg_t));

	zfree(client_create_file);
	zfree(rpc_client->recv_buff);
}

static void f_remove(fclient_t *fclient, removefile_msg_t *removefile_msg)
{
	rpc_client_t *rpc_client = NULL;
	client_remove_file_t *client_remove_file = NULL;
	int fifo_wfd;
	zclient_t *zclient = NULL;
	sds path;
	file_ret_msg_t ret_msg;

	log_write(LOG_DEBUG, "removing a file");
	//1.send message to data master to delete file
	//construct client_remove_file message
	rpc_client = fclient->rpc_client;
	client_remove_file = zmalloc(sizeof(client_remove_file_t));
	init_remove_msg(client_remove_file, removefile_msg);
	client_remove_file->unique_tag = rpc_client->tag;

	//send create message to data master
	rpc_client->op->set_send_buff(rpc_client, client_remove_file, sizeof(client_remove_file_t));
	if(rpc_client->op->execute(rpc_client, COMMAND_WITH_RETURN) < 0)
	{
		ret_msg.ret_code = FSERVER_ERR;
	}

	//2.delete znode in zserver
	path = sds_new(removefile_msg->file_path);
	zclient = fclient->zclient;
	if(zclient->op->delete_znode(zclient, path, -1) != ZOK)
	{
		ret_msg.ret_code = FSERVER_ERR;
	}
	sds_free(path);

	//copy result to share memory
	fifo_wfd = fclient->fifo_wfd;
	ret_msg.fd = -1;
	if(ret_msg.ret_code != FSERVER_ERR)
	{
		ret_msg.ret_code = code_transfer(((file_sim_ret_t*)rpc_client->recv_buff)->op_status);
	}
	write(fifo_wfd, &ret_msg, sizeof(file_ret_msg_t));

	zfree(client_remove_file);
	zfree(rpc_client->recv_buff);
}

//open a exist file. open a file do not need to sent a message to data master,
//only need to create a openedfile structure in local and get version number
//from zserver.
static void f_open(fclient_t *fclient, openfile_msg_t *openfile_msg)
{
	int fifo_wfd;
	opened_file_t *opened_file = NULL;
	list_t *file_list = NULL;
	zclient_t *zclient = NULL;
	sds path, return_data;
	znode_status_t zstatus;
	file_ret_msg_t ret_msg;

	log_write(LOG_DEBUG, "open a file to store data");

	//get znode informamtion on server
	path = sds_new(openfile_msg->file_path);
	return_data = sds_new_len(NULL, MAX_RET_DATA_LEN);
	zclient = fclient->zclient;
	if(zclient->op->get_znode(zclient, path, return_data, &zstatus, 0, NULL, NULL) != ZOK)
	{
		ret_msg.ret_code = FSERVER_ERR;
	}
	sds_free(path);
	sds_free(return_data);

	//create opened_file structure and add it to list
	file_list = fclient->file_list;
	opened_file = create_openedfile(openfile_msg->file_path, 0, openfile_msg->open_mode);
	opened_file->fd = get_fd(fclient);
	//init_file_struct(opened_file, rpc_client->recv_buff);
	opened_file->version = zstatus.version;

	file_list->list_ops->list_add_node_tail(file_list, opened_file);

	//copy result to share memory
	fifo_wfd = fclient->fifo_wfd;
	ret_msg.fd = opened_file->fd;
	if(ret_msg.ret_code != FSERVER_ERR)
	{
		ret_msg.ret_code = FOK;
	}
	write(fifo_wfd, &ret_msg, sizeof(file_ret_msg_t));
}

static void f_close(fclient_t *fclient, closefile_msg_t *closefile_msg)
{
	int fifo_wfd, fd;
	list_node_t *file_node = NULL;
	list_t *file_list = NULL;
	file_ret_msg_t ret_msg;

	log_write(LOG_DEBUG, "close a file");
	//1.find right structure
	fd = closefile_msg->fd;
	file_list = fclient->file_list;
	file_node = file_list->list_ops->list_search_key(file_list, &fd);

	//2.delete this structure form list
	file_list->list_ops->list_del_node(file_list, file_node);

	//3.free bit map
	bitmap_clear(fclient->bitmap, fd, 1);

	//4.copy return number to fifo
	fifo_wfd = fclient->fifo_wfd;
	ret_msg.fd = -1;
	ret_msg.ret_code = 0;
	write(fifo_wfd, &ret_msg, sizeof(file_ret_msg_t));
}

//write data to data_server, only support append now
static void f_append(fclient_t *fclient, appendfile_msg_t *appendfile_msg)
{
	rpc_client_t *rpc_client = NULL;
	client_append_file_t *client_append_file = NULL;
	int fifo_wfd, fd;
	opened_file_t *opened_file = NULL;
	list_t *file_list = NULL;
	zclient_t *zclient = NULL;
	pthread_mutex_t *mutex;
	sds lock_name;
	file_ret_msg_t ret_msg;

	log_write(LOG_DEBUG, "append a file to store data");
	//1.find right structure
	fd = appendfile_msg->fd;
	file_list = fclient->file_list;
	opened_file = file_list->list_ops->list_search_key(file_list, &fd)->value;

	//2.construct client_write_file message
	rpc_client = fclient->rpc_client;
	client_append_file = zmalloc(sizeof(client_append_file_t));
	init_append_msg(client_append_file, appendfile_msg, opened_file);
	client_append_file->unique_tag = rpc_client->tag;

	//3.create write lock on this server
	lock_name = sds_new_len(NULL, MAX_RET_DATA_LEN);
	zclient = fclient->zclient;
	mutex = zmalloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(mutex, NULL);
	pthread_mutex_lock(mutex);
	//add write lock, process maybe block here
	add_write_lock(zclient, opened_file->f_info.file_path, mutex, lock_name);
	//TODO why lock another time?
	pthread_mutex_lock(mutex);
	pthread_mutex_unlock(mutex);
	pthread_mutex_destroy(mutex);
	zfree(mutex);

	//4.send append message to data master
	rpc_client->op->set_send_buff(rpc_client, client_append_file, sizeof(client_append_file_t));
	if(rpc_client->op->execute(rpc_client, COMMAND_WITH_RETURN) < 0)
	{
		ret_msg.ret_code = FSERVER_ERR;
	}

	//5.copy data from shared memory and send to dataserver
	append_data(fclient, appendfile_msg, rpc_client->recv_buff);

	//6.delete lock
	zclient->op->delete_znode(zclient, lock_name, -1);

	//7.copy result to share memory
	fifo_wfd = fclient->fifo_wfd;
	ret_msg.fd = -1;
	if(ret_msg.ret_code != FSERVER_ERR)
	{
		//TODO code transfer
		ret_msg.ret_code = FOK;
	}
	write(fifo_wfd, &ret_msg, sizeof(file_ret_msg_t));

	zfree(client_append_file);
	zfree(rpc_client->recv_buff);
}

//read data from data_server
static void f_read(fclient_t *fclient, readfile_msg_t *readfile_msg)
{
	rpc_client_t *rpc_client = NULL;
	client_read_file_t *client_read_file = NULL;
	int fifo_wfd, fd;
	opened_file_t *opened_file = NULL;
	list_t *file_list = NULL;
	zclient_t *zclient = NULL;
	pthread_mutex_t *mutex;
	sds lock_name;
	file_ret_msg_t ret_msg;

	log_write(LOG_DEBUG, "read a file");
	//1.find right structure
	fd = readfile_msg->fd;
	file_list = fclient->file_list;
	opened_file = file_list->list_ops->list_search_key(file_list, &fd)->value;

	//2.construct client_read_file message
	rpc_client = fclient->rpc_client;
	client_read_file = zmalloc(sizeof(client_read_file_t));
	init_read_msg(client_read_file, readfile_msg, opened_file);
	client_read_file->unique_tag = rpc_client->tag;

	//3.create read lock on this server
	lock_name = sds_new_len(NULL, MAX_RET_DATA_LEN);
	zclient = fclient->zclient;
	mutex = zmalloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(mutex, NULL);
	pthread_mutex_lock(mutex);
	//add read lock, process maybe block here
	add_read_lock(zclient, opened_file->f_info.file_path, mutex, lock_name);
	pthread_mutex_lock(mutex);
	pthread_mutex_unlock(mutex);
	pthread_mutex_destroy(mutex);
	zfree(mutex);

	//4.send read message to data master
	rpc_client->op->set_send_buff(rpc_client, client_read_file, sizeof(client_read_file_t));
	if(rpc_client->op->execute(rpc_client, COMMAND_WITH_RETURN) < 0)
	{
		ret_msg.ret_code = FSERVER_ERR;
	}

	//5.recv data from dataserver and send to fifo
	read_data(fclient, readfile_msg, rpc_client->recv_buff);

	//6.delete lock
	zclient->op->delete_znode(zclient, lock_name, -1);

	//7.copy result to share memory
	fifo_wfd = fclient->fifo_wfd;
	ret_msg.fd = -1;
	if(ret_msg.ret_code != FSERVER_ERR){
		ret_msg.ret_code = FOK;//TODO code_transfer
	}
	write(fifo_wfd, &ret_msg, sizeof(file_ret_msg_t));

	zfree(client_read_file);
	zfree(rpc_client->recv_buff);
}

//create file client server and need to know data-master id and tag
fclient_t *create_fclient(int client_id, int target, int tag)
{
	fclient_t *fclient = zmalloc(sizeof(fclient_t));

	if(fclient == NULL)
	{
		return NULL;
	}
	fclient->rpc_client = create_rpc_client(client_id, target, tag);
	fclient->data_master_id = target;
	create_fifo(FIFO_PATH_S_TO_C, S_IRWXU);
	fclient->fifo_wfd = open_fifo(FIFO_PATH_S_TO_C, O_RDWR);
	create_fifo(FIFO_PATH_C_TO_S, S_IRWXU);
	fclient->fifo_rfd = open_fifo(FIFO_PATH_C_TO_S, O_RDWR);

	fclient->zclient = create_zclient(client_id);
	set_zclient(fclient->zclient, target, tag);
	//init bitmap and set bitmap to zero
	fclient->bitmap = (unsigned long *)zmalloc(sizeof(unsigned long) *
			MAX_FD_NUMBER / BITS_PER_LONG);
	bitmap_empty(fclient->bitmap, MAX_FD_NUMBER);
	//init file list
	fclient->file_list = list_create();
	list_set_free_method(fclient->file_list, free_file);
	list_set_match_method(fclient->file_list, match_file);

	return fclient;
}

void destroy_fclient(fclient_t *fclient)
{
	close_fifo(fclient->fifo_rfd);
	close_fifo(fclient->fifo_wfd);
	remove_fifo(FIFO_PATH_S_TO_C);
	remove_fifo(FIFO_PATH_C_TO_S);
	destroy_rpc_client(fclient->rpc_client);

	fclient->zclient->op->stop_zclient(fclient->zclient);
	destroy_zclient(fclient->zclient);

	list_release(fclient->file_list);

	zfree(fclient);
}

void *fclient_run(void *args)
{
	fclient_t *fclient = (fclient_t *)args;
	int fifo_rfd, fifo_wfd, ret = 0, client_stop = 0;
	file_msg_t file_msg;
	zclient_t *zclient = NULL;

	log_write(LOG_INFO, "===client server start run===");
	fifo_rfd = fclient->fifo_rfd;
	fifo_wfd = fclient->fifo_wfd;
	zclient = fclient->zclient;
	zclient->op->start_zclient(zclient);
	while(!client_stop)
	{
		read(fifo_rfd, &file_msg, sizeof(file_msg));
		switch(file_msg.closefile_msg.operation_code)
		{
			case FCREATE_OP:
				log_write(LOG_DEBUG, "client server handle create operation");
				f_create(fclient, &(file_msg.createfile_msg));
				break;
			case FOPEN_OP:
				log_write(LOG_DEBUG, "client server handle open operation");
				f_open(fclient, &(file_msg.openfile_msg));
				break;
			case FAPPEND_OP:
				log_write(LOG_DEBUG, "client server handle append operation");
				write(fifo_wfd, &ret, sizeof(int));
				f_append(fclient, &(file_msg.appendfile_msg));
				break;
			case FREAD_OP:
				log_write(LOG_DEBUG, "client server handle read operation");
				f_read(fclient, &(file_msg.readfile_msg));
				break;
			case FCLOSE_OP:
				log_write(LOG_DEBUG, "client server handle close operation");
				f_close(fclient, &(file_msg.closefile_msg));
				break;
			case FREMOVE_OP:
				log_write(LOG_DEBUG, "client server handle remove operation");
				f_remove(fclient, &(file_msg.removefile_msg));
				break;
			case FSTOP_OP:
				log_write(LOG_DEBUG, "client server handle stop operation");
				client_stop = 1;
				fclient->zclient->op->stop_zclient(fclient->zclient);
				break;
		}
	}
	return 0;
}
