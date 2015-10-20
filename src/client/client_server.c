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
static void init_create_msg(client_create_file_t *, createfile_msg_t *);
static void init_open_msg(client_open_file_t *, openfile_msg_t *);
static int code_transfer(uint32_t op_status);
static void f_create(fclient_t *fclient, createfile_msg_t *createfile_msg);
static int f_open(fclient_t *fclient, openfile_msg_t *openfile_msg);

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
		*client_create_file, createfile_msg_t *createfile_msg)
{
	if(createfile_msg->open_mode == CREATE_TEMP)
		client_create_file->operation_code = CREATE_TEMP_FILE_CODE;
	else
		client_create_file->operation_code = CREATE_PERSIST_FILE_CODE;;
	client_create_file->file_mode = createfile_msg->mode;
	strcpy(client_create_file->file_name, createfile_msg->file_path);
}

static void init_open_msg(client_open_file_t *client_open_file, openfile_msg_t
		*openfile_msg)
{
	client_open_file->operation_code = OPEN_FILE_CODE;
	strcpy(client_open_file->file_name, openfile_msg->file_path);
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
	zcient_t zclient = NULL;
	sds path, data;
	file_ret_msg ret_msg;

	log_write(LOG_DEBUG, "creating a file to store data");
	//construct client_create_file message
	rpc_client = fclient->rpc_client;
	client_create_file = zmalloc(sizeof(client_create_file_t));
	init_create_msg(client_create_file, createfile_msg);

	//send create message to data master
	rpc_client->op->set_send_buff(rpc_client, client_create_file,
			sizeof(client_create_file_t));
	if(rpc_client->op->execute(rpc_client, COMMAND_WITH_RETURN) < 0)
		ret_msg.ret_code = FSERVER_ERR;

	//create znode on server
	path = sds_new(createfile_msg->file_path);
	data = sds_new("ver 1")
	zclient = fcient->zclient;
	if(zcilent->op->create_parent(zcient, path, data, PERSISTENT,
				NULL) != ZOK)
		ret_msg.ret_code = FSERVER_ERR;
	sds_free(path);
	sds_free(data);

	//copy result to share memory
	shmem = fclient->shmem;
	ret_msg.fd = get_fd(fclient);
	if(ret_msg.ret_code != FSERVER_ERR)
		ret_msg.ret_code = code_transfer(((acc_msg_t
						*)rpc_client->recv_buff)->op_status);
	shmem->send_to_shm(shmem, &ret_msg, sizeof(file_ret_msg));

	zfree(client_create_file);
	zfree(rpc_client->recv_buff);

	//create opened_file structure and add it to list
	file_list = fclient->file_list;
	opened_file = create_file(createfile_msg->file_path, 0,
			createfile_msg->open_mode);
	opened_file->fd = ret_msg.fd;
	opened_file->file_info.file_len = 0;

	file_list->list_ops->list_add_node_tail(file_list, opened_file);
}

//open a exist file
static int f_open(fclient_t *fclient, openfile_msg_t *openfile_msg)
{
	rpc_client_t *rpc_cilent = NULL;
	client_open_file_t *cilent_open_file = NULL;
	shmem_t *shmem = NULL;
	opened_file_t *opened_file = NULL;
	list_t *file_list = NULL;
	zcient_t zclient = NULL;
	sds path, return_data;

	log_write(LOG_DEBUG, "open a file to store data");
	//construct client_create_file message
	rpc_client = fclient->rpc_client;
	client_open_file = zmalloc(sizeof(client_open_file_t));
	init_open_msg(client_open_file, openfile_msg);

	//send create message to data master
	rpc_client->op->set_send_buff(rpc_client, client_open_file,
			sizeof(client_open_file_t));
	if(rpc_client->op->execute(rpc_client, COMMAND_WITH_RETURN) < 0)
		ret_msg.ret_code = FSERVER_ERR;

	//create znode on server
	path = sds_new(createfile_msg->file_path);
	return_data = sds_new_len(NULL, MAX_RET_DATA_LEN);
	zclient = fcient->zclient;
	if(zcilent->op->get_znode(zclient, path, return_data, NULL, 0,
				NULL, NULL) != ZOK)
		ret_msg.ret_code = FSERVER_ERR;
	sds_free(path);

	//copy result to share memory
	shmem = fclient->shmem;
	ret_msg.fd = get_fd(fclient);
	if(ret_msg.ret_code != FSERVER_ERR)
		ret_msg.ret_code = code_transfer(((acc_msg_t
						*)rpc_client->recv_buff)->op_status);
	shmem->send_to_shm(shmem, &ret_msg, sizeof(file_ret_msg));

	zfree(client_create_file);
	zfree(rpc_client->recv_buff);

	//create opened_file structure and add it to list
	//use ret message
	file_list = fclient->file_list;
	opened_file = create_file(openfile_msg->file_path, 0,
			openfile_msg->open_mode);
	opened_file->fd = ret_msg.fd;
	opened_file->file_info.file_len = 0;
	sds_free(return_data);

	file_list->list_ops->list_add_node_tail(file_list, opened_file);
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
