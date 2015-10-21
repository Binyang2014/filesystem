/*
 * create on: 2015.1019
 * author: Binyang
 * This file will implement functions define in clinet_server.h
 */
#include <string.h>
#include "log.h"
#include "client_server.h"
#include "zmalloc.h"
#include "message.h"

static int get_fd(fclient_t *fclient);
static void init_create_msg(client_create_file_t *, const createfile_msg_t *);
static void init_open_msg(client_open_file_t *, const openfile_msg_t *);
static void init_append_msg(client_append_file_t *client_append_file, const
		writefile_msg_t *writefile_msg, const opened_file_t *opened_file);
static void init_file_struct(opened_file_t *opened_file, const file_ret_t *file_ret);
static int code_transfer(uint32_t op_status);

static int write_lock();
static int read_lock();

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
	strcpy(client_op_file->file_name, opened_file->file_info->file_path);
	client_append_file->write_size = writefile_msg->data_len;
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
//===============================ACTUAL OPERATIONS=============================
//this function is going to finish file create operation, there will be message
//passing between client and server, after it will add a open file structure to
//opened file list.
static void f_create(fclient_t *fclient, createfile_msg_t *createfile_msg)
{
	rpc_client_t *rpc_cilent = NULL;
	client_create_file_t *cilent_create_file = NULL;
	shmem_t *shmem = NULL;
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
	shmem = fclient->shmem;
	ret_msg.fd = opened_file->fd;
	if(ret_msg.ret_code != FSERVER_ERR)
		ret_msg.ret_code = code_transfer(((acc_msg_t
						*)rpc_client->recv_buff)->op_status);
	shmem->send_to_shm(shmem, &ret_msg, sizeof(file_ret_msg));

	zfree(client_create_file);
	zfree(rpc_client->recv_buff);
}

//open a exist file
static void f_open(fclient_t *fclient, openfile_msg_t *openfile_msg)
{
	rpc_client_t *rpc_cilent = NULL;
	client_open_file_t *cilent_open_file = NULL;
	shmem_t *shmem = NULL;
	opened_file_t *opened_file = NULL;
	list_t *file_list = NULL;
	zcient_t *zclient = NULL;
	sds path, return_data;
	znode_status_t zstatus;

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
	shmem = fclient->shmem;
	ret_msg.fd = opened_file->fd;
	if(ret_msg.ret_code != FSERVER_ERR)
		ret_msg.ret_code = code_transfer(file_ret->op_status);
	shmem->send_to_shm(shmem, &ret_msg, sizeof(file_ret_msg));

	zfree(client_create_file);
	zfree(rpc_client->recv_buff);
}

//write data to data_server, only support append now
static void f_append(fclient_t *fclient, writefile_msg_t *writefile_msg, int fd)
{
	rpc_client_t *rpc_cilent = NULL;
	client_append_file_t *cilent_append_file = NULL;
	shmem_t *shmem = NULL;
	opened_file_t *opened_file = NULL;
	list_t *file_list = NULL;
	zcient_t *zclient = NULL;
	sds path, return_data;

	log_write(LOG_DEBUG, "append a file to store data");
	//1.find right structure
	file_list = fclient->file_list;
	opened_file = file_list->list_ops->list_search_key(file_list, &fd)->value;

	//2.construct client_create_file message
	rpc_client = fclient->rpc_client;
	client_append_file = zmalloc(sizeof(client_append_file_t));
	init_append_msg(client_open_file, writefile_msg, opened_file);
	client_append_msg->unique_tag = rpc_client->tag;

	//3.send create message to data master
	rpc_client->op->set_send_buff(rpc_client, client_open_file,
			sizeof(client_open_file_t));
	if(rpc_client->op->execute(rpc_client, COMMAND_WITH_RETURN) < 0)
		ret_msg.ret_code = FSERVER_ERR;

	//4.create write lock on this server

	//5.copy data from shared memory and send to dataserver

	//6.copy result to share memory

	zfree(client_create_file);
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
	fclient->shmem = create_shm(COMMON_KEY, MAX_SHM_SIZE, SHM_UREAD | SHM_UWRITE);
	if(fclient->shmem == NULL)
	{
		destroy_rpc_client(fclient->rpc_client);
		zfree(fclient);
		return NULL;
	}
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
	destroy_shm(fclient->shmem);
	destroy_rpc_client(fclient->rpc_client);

	fclient->zcient->op->stop_zclient(zcilent);
	destroy_zclient(fclient->zclient);

	list_release(fclient->file_list);
	zfree(fclient->fclient_ops);

	zfree(fclient);
}
