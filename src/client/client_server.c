/*
 * create on: 2015.1019
 * author: Binyang
 * This file will implement functions define in clinet_server.h
 */
#include <string.h>
#include <pthread.h>
#include "log.h"
#include "client_server.h"
#include "zmalloc.h"
#include "message.h"

static int get_fd(fclient_t *fclient);
static void init_create_msg(client_create_file_t *, const createfile_msg_t *);
static void init_open_msg(client_open_file_t *, const openfile_msg_t *);
static void init_append_msg(client_append_file_t *, const writefile_msg_t *,
		const opened_file_t *);
static void init_read_msg(client_read_file_t *, const readfile_msg_t *, const
		opened_file_t *);
static void init_file_struct(opened_file_t *opened_file, const file_ret_t *file_ret);
static int code_transfer(uint32_t op_status);

static int add_write_lock(zclient_t *zclient, const char *file_path,
		pthread_mutex_t *mutex);
static int add_read_lock(zclient_t *zclient, const char *file_path,
		pthread_mutex_t *mutex);
static void *watch_handler_delete(void *args);


static int append_data(fclient_t *fclient, writefile_msg_t *writefile_msg,
		file_ret_t *file_ret);
static int read_data(fclient_t *fclient, readfile_msg_t *readfile_msg,
		file_ret_t *file_ret);

static void f_create(fclient_t *fclient, createfile_msg_t *createfile_msg);
static void f_open(fclient_t *fclient, openfile_msg_t *openfile_msg);
static void f_append(fclient_t *fclient, writefile_msg_t *writefile_msg);
static void f_read(fclient *fclient, readfile_msg_t *readfile_msg);

//=======================UTILITY FUNCTIONS======================
static int code_transfer(uint32_t op_status)
{
	switch(op_status)
	{
		case FILE_NOT_EXIST
			return FNO_EXISIT;
		default:
			return FOK;
	}
	return FOK;
}

static int get_fd(fclient_t *fclient)
{
	int fd;

	fd = find_first_zero_bit(f_client->bitmap, MAX_FD_NUMBER);
	bitmap_set(f_client->bitmap, fd, 1);
	return fd;
}

static void init_create_msg(client_create_file_t
		*client_create_file, const createfile_msg_t *createfile_msg)
{
	if(createfile_msg->open_mode == CREATE_TEMP)
		client_create_file->operation_code = CREATE_TEMP_FILE_CODE;
	else
		client_create_file->operation_code = CREATE_PERSIST_FILE_CODE;;
	client_create_file->file_mode = createfile_msg->mode;
	strcpy(client_create_file->file_name, createfile_msg->file_path);
}

static void init_open_msg(client_open_file_t *client_open_file, const openfile_msg_t
		*openfile_msg)
{
	client_open_file->operation_code = OPEN_FILE_CODE;
	strcpy(client_open_file->file_name, openfile_msg->file_path);
}

static void init_append_msg(client_append_file_t *client_append_file, const
		writefile_msg_t *writefile_msg, const opened_file_t *opened_file)
{
	client_append_file->operation_code = APPEND_FILE_CODE;
	strcpy(client_append_file->file_name, opened_file->file_info->file_path);
	client_append_file->write_size = writefile_msg->data_len;
}

static void init_read_msg(client_read_file_t * client_read_file, const
		readfile_msg_t * readfile_msg, const opened_file_t *opened_file)
{
	client_read_file->operation_code = READ_FILE_CODE;
	strcpy(client_read_file->file_name, opened_file->file_info->file_path);
	client_read_file->read_size = read_msg->data_len;
	client_read_file->offset = read_msg->offset;
}

static void init_file_struct(opened_file_t *opened_file, const file_ret_t *file_ret)
{
	int dataserver_num, chunks_num, index = 0;
	int i, j;
	list_t *data_nodes = opened_file->file_info.data_nodes;
	data_node_t *data_node;

	dataserver_num = file_ret->dataserver_num;
	chunks_num = file_ret->chunks_num;
	opened_file->file_info.file_len = file_ret->file_size;
	opened_file->file_info.file_offset = file_ret->offset;
	for(i = 0; i < dataserver_num; i++)
	{
		data_node = zmalloc(sizeof(data_node_t));
		data_node->data_server_id = file_ret->data_server_arr[i];
		data_node->chunks_id = zmalloc(sizeof(uint64_t) * file_ret->chunks_num);
		for(j = 0; j < data_server_cnum[i], j++)
			data_node->chunks_id[j] = file_ret->chunks_id_arr[index++];
		data_nodes->list_ops->list_add_node_tail(data_nodes, data_node);
	}
}

static int add_write_lock(zclient_t *zclient, const char *file_path,
		pthread_mutex_t *mutex, sds lock_name)
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
	ret_num = zclient->op->create_znode(zclient, path, data,
			EPHEMERAL_SQUENTIAL, return_name);
	//copy lock name
	lock_name = sds_cpy(lock_name, return_name);
	path = sds_cpy(path, file_path);
	path = sds_cat(path, ":lock");
	ret_num = zclient->op->get_children(zclient, path, return_data);
	children = sds_split_len(return_data, strlen(return_data), " ", 1, &count);
	//no more other lock
	if(count == 1)
		pthread_mutex_unlock(mutex);
	//there are other locks
	else
	{
		begin = return_name + strlen(return_name);
		while(*begin != '/')
			begin--;
		begin = begin + 1;

		for(i = 0; i < count; i++)
		{
			if(strcpy(children[i], begin) == 0)
				break;
		}
		path = sds_cpy(path, file_path);
		path = sds_cat(path, ":lock/");
		path = sds_cat(path, children[i - 1]);
		zclient->op->exists_znode(zclient, path, NULL, NOTICE_DELETE,
				watch_handler_delete, mutex);
	}

	sds_free(path);
	sds_free(data);
	sds_free(return_data);
	sds_free(return_name);
	sds_free_split_res(children, count);
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
	ret_num = zclient->op->create_znode(zclient, path, data,
			EPHEMERAL_SQUENTIAL, return_name);
	//copy lock name
	lock_name = sds_cpy(lock_name, return_name);
	path = sds_cpy(path, file_path);
	path = sds_cat(path, ":lock");
	ret_num = zclient->op->get_children(zclient, path, return_data);
	children = sds_split_len(return_data, strlen(return_data), " ", 1, &count);
	//no more other lock
	if(count == 1)
		pthread_mutex_unlock(mutex);
	//there are other locks
	else
	{
		begin = return_name + strlen(return_name);
		while(*begin != '/')
			begin--;
		begin = begin + 1;

		for(i = 0; i < count; i++)
		{
			if(strcpy(children[i], begin) == 0)
				break;
		}
		for(j = i - 1; j > 0; j--)
		{
			if(children[j][0] == 'w')
				break;
		}
		if(j < 0)
			pthread_mutex_unlock(mutex);
		else
		{
			path = sds_cpy(path, file_path);
			path = sds_cat(path, ":lock/");
			path = sds_cat(path, children[j]);
			zclient->op->exists_znode(zclient, path, NULL, NOTICE_DELETE,
					watch_handler_delete, mutex);
		}
	}

	sds_free(path);
	sds_free(data);
	sds_free(return_data);
	sds_free(return_name);
	sds_free_split_res(children, count);
}
static void *watch_handler_delete(void *args)
{
	pthread_mutex_t *mutex;

	mutex = args;
	pthread_mutex_unlock(mutex);
}


static int append_data(fclient_t *fclient, appendfile_msg_t *appendfile_msg,
		file_ret_t *file_ret)
{
	int dataserver_num, chunks_num;
	int i, j, index = 0, ret;
	rpc_client_t *rpc_client;
	write_c_to_d_t write_msg;
	size_t write_len;
	char *data_msg;

	dataserver_num = file_ret->dataserver_num;
	rpc_client = fclient->rpc_client;
	write_len = appendfile_msg->data_len;
	write_msg.operation_code = C_D_WRITE_BLOCK_CODE;
	write_msg.unique_tag = rpc_client->tag;
	for(i = 0; i < dataserver_num; i++)
	{
		int server_id = data_server_arr[i];

		rpc_client->target = server_id;
		write_msg.offset = file_ret->data_server_offset[i];
		write_msg.write_len = file_ret->data_server_len[i];
		data_msg = zmalloc(write_msg.write_len);
		read(fclient->fd, data_msg, write_msg.write_len);
		write_msg.chunks_count = file_ret->data_server_cnum[i];

		for(j = 0; j < write_msg.chunks_count; j++)
			write_msg.chunks_id_arr[j] = file_ret_t->chunks_id_arr[index++];

		rpc_client->op->set_send_buff(rpc_client, &write_msg,
				sizeof(write_c_to_d_t));
		rpc_client->op->set_second_send_buff(client, data_msg,
				write_msg.write_len);
		ret = rpc_client->op->execute(rpc_client, WRITE_C_TO_D);
		if(ret < 0)
		{
			zfree(data_msg);
			rpc_client->target = fclient->data_master_id;
			return -1;
		}
		zfree(data_msg);
	}
	rpc_client->target = fclient->data_master_id;

	return 0;
}

static int read_data(fclient_t *fclient, readfile_msg_t *readfile_msg,
		file_ret_t *file_ret)
{
	int dataserver_num, chunks_num;
	int i, j, index = 0, ret;
	rpc_client_t *rpc_client;
	read_c_to_d_t read_msg;
	size_t read_len;
	char *data_msg;

	dataserver_num = file_ret->dataserver_num;
	rpc_client = fclient->rpc_client;
	read_len = readfile_msg->data_len;
	read_msg.operation_code = C_D_WRITE_BLOCK_CODE;
	read_msg.unique_tag = rpc_client->tag;
	for(i = 0; i < dataserver_num; i++)
	{
		int server_id = data_server_arr[i];

		rpc_client->target = server_id;
		read_msg.offset = file_ret->data_server_offset[i];
		read_msg.read_len = file_ret->data_server_len[i];
		data_msg = zmalloc(read_msg.read_len);
		read_msg.chunks_count = file_ret->data_server_cnum[i];

		for(j = 0; j < read_msg.chunks_count; j++)
			read_msg.chunks_id_arr[j] = file_ret_t->chunks_id_arr[index++];

		rpc_client->op->set_send_buff(rpc_client, &read_msg,
				sizeof(read_c_to_d_t));
		rpc_client->op->set_second_send_buff(client, data_msg,
				read_msg.write_len);
		ret = rpc_client->op->execute(rpc_client, READ_C_TO_D);
		write(fclient->fd, data_msg, read_msg.read_len);
		if(ret < 0)
		{
			zfree(data_msg);
			rpc_client->target = fclient->data_master_id;
			return -1;
		}
		zfree(data_msg);
	}
	rpc_client->target = fclient->data_master_id;

	return 0;
}
//===============================ACTUAL OPERATIONS=============================
//this function is going to finish file create operation, there will be message
//passing between client and server, after it will add a open file structure to
//opened file list.
static void f_create(fclient_t *fclient, createfile_msg_t *createfile_msg)
{
	rpc_client_t *rpc_cilent = NULL;
	client_create_file_t *cilent_create_file = NULL;
	int fd;
	opened_file_t *opened_file = NULL;
	list_t *file_list = NULL;
	zcient_t *zclient = NULL;
	sds path, data;
	file_ret_msg ret_msg;

	log_write(LOG_DEBUG, "creating a file to store data");
	//construct client_create_file message
	rpc_client = fclient->rpc_client;
	client_create_file = zmalloc(sizeof(client_create_file_t));
	init_create_msg(client_create_file, createfile_msg);
	client_create_msg->unique_tag = rpc_client->tag;

	//send create message to data master
	rpc_client->op->set_send_buff(rpc_client, client_create_file,
			sizeof(client_create_file_t));
	if(rpc_client->op->execute(rpc_client, COMMAND_WITH_RETURN) < 0)
		ret_msg.ret_code = FSERVER_ERR;

	//create znode on server
	path = sds_new(createfile_msg->file_path);
	//have not used data structure now
	data = sds_new("msg:")
	zclient = fcient->zclient;
	if(zcilent->op->create_parent(zcient, path, data, PERSISTENT,
				NULL) != ZOK)
		ret_msg.ret_code = FSERVER_ERR;
	//create lock for this file
	path = sds_cat(path, ":lock");
	data = sds_cpy(data, "This is a lock");
	zclient->op->create_parent(zclient, path, "", PERSISTENT, NULL);
	sds_free(path);
	sds_free(data);

	//create opened_file structure and add it to list
	file_list = fclient->file_list;
	opened_file = create_file(createfile_msg->file_path, 0,
			createfile_msg->open_mode);
	opened_file->fd = get_fd(fclient);
	opened_file->file_info.file_len = 0;

	file_list->list_ops->list_add_node_tail(file_list, opened_file);

	//copy result to share memory
	fd = fclient->fd;
	ret_msg.fd = opened_file->fd;
	if(ret_msg.ret_code != FSERVER_ERR)
		ret_msg.ret_code = code_transfer(((acc_msg_t
						*)rpc_client->recv_buff)->op_status);
	write(fd, &ret_msg, sizeof(file_ret_msg));

	zfree(client_create_file);
	zfree(rpc_client->recv_buff);
}

//open a exist file
static void f_open(fclient_t *fclient, openfile_msg_t *openfile_msg)
{
	rpc_client_t *rpc_cilent = NULL;
	client_open_file_t *cilent_open_file = NULL;
	int fd;
	opened_file_t *opened_file = NULL;
	list_t *file_list = NULL;
	zcient_t *zclient = NULL;
	sds path, return_data;
	znode_status_t zstatus;
	file_ret_msg ret_msg;

	log_write(LOG_DEBUG, "open a file to store data");
	//construct client_create_file message
	rpc_client = fclient->rpc_client;
	client_open_file = zmalloc(sizeof(client_open_file_t));
	init_open_msg(client_open_file, openfile_msg);
	client_open_msg->unique_tag = rpc_client->tag;

	//send create message to data master
	rpc_client->op->set_send_buff(rpc_client, client_open_file,
			sizeof(client_open_file_t));
	if(rpc_client->op->execute(rpc_client, COMMAND_WITH_RETURN) < 0)
		ret_msg.ret_code = FSERVER_ERR;

	//get znode informamtion on server
	path = sds_new(createfile_msg->file_path);
	return_data = sds_new_len(NULL, MAX_RET_DATA_LEN);
	zclient = fcient->zclient;
	if(zcilent->op->get_znode(zclient, path, return_data, &zstatus, 0,
				NULL, NULL) != ZOK)
		ret_msg.ret_code = FSERVER_ERR;
	sds_free(path);
	sds_free(return_data);

	//create opened_file structure and add it to list
	file_list = fclient->file_list;
	opened_file = create_file(openfile_msg->file_path, 0,
			openfile_msg->open_mode);
	opened_file->fd = get_fd(fclient);
	init_file_struct(opened_file, rpc_client->recv_buff);
	opened_file->version = zstatus.version;

	file_list->list_ops->list_add_node_tail(file_list, opened_file);

	//copy result to share memory
	fd = fclient->fd;
	ret_msg.fd = opened_file->fd;
	if(ret_msg.ret_code != FSERVER_ERR)
		ret_msg.ret_code = code_transfer(rpc_client->recv_buff->op_status);
	write(fd, &ret_msg, sizeof(file_ret_msg));

	zfree(client_create_file);
	zfree(rpc_client->recv_buff);
}

//write data to data_server, only support append now
static void f_append(fclient_t *fclient, appendfile_msg_t *appendfile_msg, int fd)
{
	rpc_client_t *rpc_cilent = NULL;
	client_append_file_t *cilent_append_file = NULL;
	int fifo_fd;
	opened_file_t *opened_file = NULL;
	list_t *file_list = NULL;
	zcient_t *zclient = NULL;
	pthread_mutex_t *mutex;
	sds lock_name;
	file_ret_msg ret_msg;

	log_write(LOG_DEBUG, "append a file to store data");
	//1.find right structure
	file_list = fclient->file_list;
	opened_file = file_list->list_ops->list_search_key(file_list, &fd)->value;

	//2.construct client_write_file message
	rpc_client = fclient->rpc_client;
	client_append_file = zmalloc(sizeof(client_append_file_t));
	init_append_msg(client_open_file, appendfile_msg, opened_file);
	client_append_msg->unique_tag = rpc_client->tag;

	//3.create write lock on this server
	lock_name = sds_new_len(NULL, MAX_RET_DATA_LEN);
	zclient = fclient->zclient;
	mutex = zmalloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(mutex, NULL);
	pthread_mutex_lock(mutex);
	//add write lock, process maybe block here
	add_write_lock(zclient, opened_file->f_info.file_path, mutex, lock_name);
	pthread_mutex_lock(mutex);
	thread_mutex_unlock(mutex);
	pthread_mutex_destroy(mutex);
	zfree(mutex);

	//4.send append message to data master
	rpc_client->op->set_send_buff(rpc_client, client_append_file,
			sizeof(client_append_file_t));
	if(rpc_client->op->execute(rpc_client, COMMAND_WITH_RETURN) < 0)
		ret_msg.ret_code = FSERVER_ERR;

	//5.copy data from shared memory and send to dataserver
	append_data(fclient, appendfile_msg, rpc_client->recv_buff);

	//6.delete lock
	zclient->op->delete_znode(zcient, path, -1);

	//7.copy result to share memory
	fifo_fd = fclient->fd;
	ret_msg.fd = -1;
	if(ret_msg.ret_code != FSERVER_ERR)
		ret_msg.ret_code = code_transfer(rpc_client->recv_buff->op_status);
	write(fifo_fd, &ret_msg, sizeof(file_ret_msg));

	zfree(client_append_file);
	zfree(rpc_client->recv_buff);
}

//read data from data_server
static void f_read(fclient_t *fclient, readfile_msg_t *readfile_msg, int fd)
{
	rpc_client_t *rpc_cilent = NULL;
	client_read_file_t *cilent_read_file = NULL;
	int fifo_fd;
	opened_file_t *opened_file = NULL;
	list_t *file_list = NULL;
	zcient_t *zclient = NULL;
	pthread_mutex_t *mutex;
	sds lock_name;
	file_ret_msg ret_msg;

	log_write(LOG_DEBUG, "append a file to store data");
	//1.find right structure
	file_list = fclient->file_list;
	opened_file = file_list->list_ops->list_search_key(file_list, &fd)->value;

	//2.construct client_read_file message
	rpc_client = fclient->rpc_client;
	client_read_file = zmalloc(sizeof(client_read_file_t));
	init_read_msg(client_read_file, readfile_msg, opened_file);
	client_read_msg->unique_tag = rpc_client->tag;

	//3.create read lock on this server
	lock_name = sds_new_len(NULL, MAX_RET_DATA_LEN);
	zclient = fclient->zclient;
	mutex = zmalloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(mutex, NULL);
	pthread_mutex_lock(mutex);
	//add read lock, process maybe block here
	add_read_lock(zclient, opened_file->f_info.file_path, mutex, lock_name);
	pthread_mutex_lock(mutex);
	thread_mutex_unlock(mutex);
	pthread_mutex_destroy(mutex);
	zfree(mutex);

	//4.send read message to data master
	rpc_client->op->set_send_buff(rpc_client, client_read_file,
			sizeof(client_read_file_t));
	if(rpc_client->op->execute(rpc_client, COMMAND_WITH_RETURN) < 0)
		ret_msg.ret_code = FSERVER_ERR;

	//5.recv data from dataserver and send to fifo
	read_data(fclient, readfile_msg, rpc_client->recv_buff);

	//6.delete lock
	zclient->op->delete_znode(zcient, path, -1);

	//7.copy result to share memory
	fd = fclient->fd;
	ret_msg.fd = -1;
	if(ret_msg.ret_code != FSERVER_ERR)
		ret_msg.ret_code = code_transfer(rpc_client->recv_buff->op_status);
	write(fd, &ret_msg, sizeof(file_ret_msg));

	zfree(client_read_file);
	zfree(rpc_client->recv_buff);
}

//create file client server and need to know data-master id and tag
fclient_t *create_fclient(int client_id, int target, int tag)
{
	fclient_t *fclient = zmalloc(sizeof(fclient_t));

	if(fclient == NULL)
		return NULL;
	fclient->rpc_client = create_rpc_client(client_id, target, tag);
	fclient->data_master_id = target;
	create_fifo(FIFO_PATH, S_IRWXU);
	fclient->fd = open_fifo(FIFO_PATH, O_RDWR);

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

	fclient->fclient_ops = zmalloc(sizeof(fclient_ops_t));
	fclient->fclient_ops->f_create = f_create;

	return fclient;
}

void destroy_fclient(fclient_t *fclient)
{
	fifo_close(fclient->fd);
	remove_fifo(FIFO_PATH);
	destroy_rpc_client(fclient->rpc_client);

	fclient->zcient->op->stop_zclient(zcilent);
	destroy_zclient(fclient->zclient);

	list_release(fclient->file_list);
	zfree(fclient->fclient_ops);

	zfree(fclient);
}
