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

extern void system_test(int rank);
static fclient_t *fclient;
static int get_fd(fclient_t *fclient);
static void init_create_msg(client_create_file_t *, const createfile_msg_t *);
static void init_append_msg(client_append_file_t *, const appendfile_msg_t *, const opened_file_t *);
static void init_read_msg(client_read_file_t *, const readfile_msg_t *, const opened_file_t *);
static void init_remove_msg(client_remove_file_t *, const removefile_msg_t *);
//static void init_file_struct(opened_file_t *opened_file, const file_ret_t *file_ret);
static int code_transfer(uint32_t op_status);

static int add_write_lock(zclient_t *zclient, const char *file_path, pthread_mutex_t *mutex, sds lock_name);
static int add_read_lock(zclient_t *zclient, const char *file_path, pthread_mutex_t *mutex, sds lock_name);
static void *watch_handler_delete(void *args);


static int append_data(fclient_t *fclient, appendfile_msg_t *writefile_msg, void
		*file_ret, const char *data);
static int read_data(fclient_t *fclient, readfile_msg_t *readfile_msg, void
		*file_ret, char *data);

int fs_create(createfile_msg_t *createfile_msg, int *ret_fd);
int fs_open(openfile_msg_t *openfile_msg, int *ret_fd);
int fs_append(appendfile_msg_t *writefile_msg, const char *data);
int fs_read(readfile_msg_t *readfile_msg, char *data);
int fs_close(closefile_msg_t *closefile_msg);
int fs_remove(removefile_msg_t *removefile_msg);

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
	client_read_file->offset = readfile_msg->offset;
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
	ret_num = zclient->op->create_parent(zclient, path, data, PERSISTENT_SQUENTIAL, return_name);
	//copy lock name
	lock_name = sds_cpy(lock_name, return_name);
	path = sds_cpy(path, file_path);
	path = sds_cat(path, ":lock");
	ret_num = zclient->op->get_children(zclient, path, return_data);
	children = sds_split_len(return_data, strlen(return_data), " ", 1, &count);
	log_write(DEBUG, "lock name is %s, count number is %d", lock_name, count);
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
		log_write(DEBUG, "begin path is %s", children[i]);

		for(i = 0; i < count; i++)
		{
			log_write(DEBUG, "children path is %s", children[i]);
			if(strcmp(children[i], begin) == 0)
			{
				break;
			}
		}
		//it is not first element
		if(i != 0)
		{
			int ret_num;
			path = sds_cpy(path, file_path);
			path = sds_cat(path, ":lock/");
			path = sds_cat(path, children[i - 1]);
			log_write(DEBUG, "watch path is %s", path);
			ret_num = zclient->op->exists_znode(zclient, path, NULL, NOTICE_DELETE, watch_handler_delete, mutex);
			if(ret_num != ZOK)
			{
				log_write(DEBUG, "release path first is %s", lock_name);
				pthread_mutex_unlock(mutex);
			}
		}
		else
		{
			log_write(DEBUG, "release path second is %s", lock_name);
			pthread_mutex_unlock(mutex);
		}
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
	ret_num = zclient->op->create_parent(zclient, path, data, PERSISTENT_SQUENTIAL, return_name);
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
			if(strcmp(children[i], begin) == 0)
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
			int ret_num;
			path = sds_cpy(path, file_path);
			path = sds_cat(path, ":lock/");
			path = sds_cat(path, children[j]);
			ret_num = zclient->op->exists_znode(zclient, path, NULL, NOTICE_DELETE, watch_handler_delete, mutex);
			if(ret_num != ZOK)
			{
				pthread_mutex_unlock(mutex);
			}
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

static int append_data(fclient_t *fclient, appendfile_msg_t *appendfile_msg,
		void *pos, const char *data)
{
	int dataserver_num;
	int i, j, ret;
	rpc_client_t *rpc_client;
	write_c_to_d_t write_msg;
	uint64_t *head;
	uint64_t write_size;
	file_ret_t *file_ret;
	position_des_t *position;
	const char *data_msg;

	head = (uint64_t *)pos;
	file_ret = (file_ret_t *)(head + 1);//TODO not hard code here
	write_size = file_ret->read_write_len;
	position = (position_des_t *)(head + 4);
	dataserver_num = file_ret->dataserver_num;
	rpc_client = fclient->rpc_client;
	write_msg.operation_code = C_D_WRITE_BLOCK_CODE;
	write_msg.unique_tag = rpc_client->tag;
	write_msg.offset = file_ret->offset;
	data_msg = data;

	for(i = 0; i < dataserver_num; i++)
	{
		position_des_t *posi = position + i;
		int server_id = posi->rank;

		rpc_client->target = server_id;
		write_msg.chunks_count = posi->end - posi->start + 1;
		write_msg.write_len = BLOCK_SIZE * (write_msg.chunks_count) - write_msg.offset;
		if(write_size < write_msg.write_len)
		{
			write_msg.write_len = write_size;
		}
		write_size -= write_msg.write_len;

		for(j = posi->start; j <= posi->end; j++)
		{
			write_msg.chunks_id_arr[j - posi->start] = j;
		}

		rpc_client->op->set_send_buff(rpc_client, &write_msg, sizeof(write_c_to_d_t));
		rpc_client->op->set_second_send_buff(rpc_client, data_msg, write_msg.write_len);
		ret = rpc_client->op->execute(rpc_client, WRITE_C_TO_D);
		data_msg = data_msg + write_msg.write_len;
		if(ret < 0)
		{
			rpc_client->target = fclient->data_master_id;
			return -1;
		}
		write_msg.offset = 0;
	}
	rpc_client->target = fclient->data_master_id;

	return 0;
}

static int read_data(fclient_t *fclient, readfile_msg_t *readfile_msg, void *pos, char *data)
{
	int dataserver_num;
	int i, j, ret;
	rpc_client_t *rpc_client;
	read_c_to_d_t read_msg;
	char *data_msg;
	uint64_t *head;
	uint64_t read_size;
	file_ret_t *file_ret;
	position_des_t *position;

	head = (uint64_t *)pos;
	file_ret = (file_ret_t *)(head + 1);
	read_size = file_ret->read_write_len;
	position = (position_des_t *)(head + 4);
	dataserver_num = file_ret->dataserver_num;
	rpc_client = fclient->rpc_client;
	read_msg.operation_code = C_D_READ_BLOCK_CODE;
	read_msg.unique_tag = rpc_client->tag;
	read_msg.offset = file_ret->offset;
	printf("\n********** offset  = %llu *****************\n", (unsigned long long)file_ret->offset);
	//read data and copy data to data
	data_msg = data;
	for(i = 0; i < dataserver_num; i++)
	{
		position_des_t *posi = position + i;
		int server_id = posi->rank;

		rpc_client->target = server_id;

		read_msg.chunks_count = posi->end - posi->start + 1;
		read_msg.read_len = read_msg.chunks_count * BLOCK_SIZE - read_msg.offset;
		if(read_size < read_msg.read_len)
		{
			read_msg.read_len = read_size;
		}
		read_size -= read_msg.read_len;
#if CLIENT_DEBUG
		log_write(LOG_DEBUG, "client server read data size = %d, count = %d, offset = %d", read_msg.read_len, read_msg.chunks_count, read_msg.offset);
#endif
		
		for(j = posi->start; j <= posi->end; j++)
		{
			read_msg.chunks_id_arr[j - posi->start] = j;
		}

		rpc_client->op->set_send_buff(rpc_client, &read_msg, sizeof(read_c_to_d_t));
		rpc_client->op->set_recv_buff(rpc_client, data_msg, read_msg.read_len);
		ret = rpc_client->op->execute(rpc_client, READ_C_TO_D);
		data_msg = data_msg + read_msg.read_len;
		if(ret < 0)
		{
			rpc_client->op->set_recv_buff(rpc_client, pos, sizeof(file_ret) +
					sizeof(uint64_t) + dataserver_num * sizeof(position_des_t));
			rpc_client->target = fclient->data_master_id;
			return -1;
		}
		read_msg.offset = 0;
	}
	rpc_client->op->set_recv_buff(rpc_client, pos, sizeof(file_ret) +
			sizeof(uint64_t) + dataserver_num * sizeof(position_des_t));
	rpc_client->target = fclient->data_master_id;

	return 0;
}
//===============================ACTUAL OPERATIONS=============================
//this function is going to finish file create operation, there will be message
//passing between client and server, after it will add a open file structure to
//opened file list.
int fs_create(createfile_msg_t *createfile_msg, int *ret_fd)
{
	rpc_client_t *rpc_client = NULL;
	client_create_file_t *client_create_file = NULL;
	opened_file_t *opened_file = NULL;
	list_t *file_list = NULL;
	zclient_t *zclient = NULL;
	sds path, data;
	int ret_code = FOK;

	log_write(LOG_DEBUG, "creating a file to store data");
	//construct client_create_file message
	rpc_client = fclient->rpc_client;
	client_create_file = zmalloc(sizeof(client_create_file_t));
	init_create_msg(client_create_file, createfile_msg);
	client_create_file->unique_tag = rpc_client->tag;
	client_create_file->source = rpc_client->client_id;

	//send create message to data master
	rpc_client->op->set_send_buff(rpc_client, client_create_file, sizeof(client_create_file_t));
	if(rpc_client->op->execute(rpc_client, COMMAND_WITH_RETURN) < 0)
	{
		ret_code = FSERVER_ERR;
	}
	
#if CLIENT_DEBUG
	log_write(LOG_DEBUG, "client server create temp file success");
#endif
	//create znode on server
	path = sds_new(createfile_msg->file_path);
	//have not used data structure now
	data = sds_new("msg:");
	zclient = fclient->zclient;
	if(zclient->op->create_parent(zclient, path, data, PERSISTENT, NULL) != ZOK)
	{
		ret_code = FSERVER_ERR;
	}
	//create lock for this file
	path = sds_cat(path, ":lock");
	data = sds_cpy(data, "This is a lock");
	zclient->op->create_parent(zclient, path, "", PERSISTENT, NULL);
	sds_free(path);
	sds_free(data);
#if CLIENT_DEBUG
	log_write(LOG_DEBUG, "client server create lock success");
#endif
	//create opened_file structure and add it to list
	file_list = fclient->file_list;
	opened_file = create_openedfile(createfile_msg->file_path, 0, createfile_msg->open_mode);
	opened_file->fd = get_fd(fclient);
	opened_file->f_info.file_len = 0;

	file_list->list_ops->list_add_node_tail(file_list, opened_file);

	zfree(client_create_file);
	zfree(rpc_client->recv_buff);

	//copy result to fd
	*ret_fd = opened_file->fd;
	if(ret_code != FSERVER_ERR)
	{
		ret_code = code_transfer(((file_sim_ret_t*)rpc_client->recv_buff)->op_status);
	}
	return ret_code;
}

int fs_remove(removefile_msg_t *removefile_msg)
{
	rpc_client_t *rpc_client = NULL;
	client_remove_file_t *client_remove_file = NULL;
	zclient_t *zclient = NULL;
	sds path;
	int ret_code = FOK;

	log_write(LOG_DEBUG, "removing a file");
	//1.send message to data master to delete file
	//construct client_remove_file message
	rpc_client = fclient->rpc_client;
	client_remove_file = zmalloc(sizeof(client_remove_file_t));
	init_remove_msg(client_remove_file, removefile_msg);
	client_remove_file->unique_tag = rpc_client->tag;
	//client_remove_file->

	//send create message to data master
	rpc_client->op->set_send_buff(rpc_client, client_remove_file, sizeof(client_remove_file_t));
	if(rpc_client->op->execute(rpc_client, COMMAND_WITH_RETURN) < 0)
	{
		ret_code = FSERVER_ERR;
	}

	//2.delete znode in zserver
	path = sds_new(removefile_msg->file_path);
	zclient = fclient->zclient;
	if(zclient->op->delete_znode(zclient, path, -1) != ZOK)
	{
		ret_code = FSERVER_ERR;
	}
	sds_free(path);

	//copy result to share memory
	if(ret_code != FSERVER_ERR)
	{
		ret_code = code_transfer(((file_sim_ret_t*)rpc_client->recv_buff)->op_status);
	}

	zfree(client_remove_file);
	zfree(rpc_client->recv_buff);
	return ret_code;
}

//open a exist file. open a file do not need to sent a message to data master,
//only need to create a openedfile structure in local and get version number
//from zserver.
int fs_open(openfile_msg_t *openfile_msg, int *ret_fd)
{
	opened_file_t *opened_file = NULL;
	list_t *file_list = NULL;
	zclient_t *zclient = NULL;
	sds path, return_data;
	znode_status_t zstatus;
	int ret_code = FOK;

	log_write(LOG_DEBUG, "open a file to store data");

	//get znode informamtion on server
	path = sds_new(openfile_msg->file_path);
	return_data = sds_new_len(NULL, MAX_RET_DATA_LEN);
	zclient = fclient->zclient;
	if(zclient->op->get_znode(zclient, path, return_data, &zstatus, 0, NULL, NULL) != ZOK)
	{
		ret_code = FSERVER_ERR;
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
	*ret_fd = opened_file->fd;
	return ret_code;
}

int fs_close(closefile_msg_t *closefile_msg)
{
	int fd;
	list_node_t *file_node = NULL;
	list_t *file_list = NULL;

	log_write(LOG_DEBUG, "close a file");
	//1.find right structure
	fd = closefile_msg->fd;
	file_list = fclient->file_list;
	file_node = file_list->list_ops->list_search_key(file_list, &fd);

	//2.delete this structure form list
	file_list->list_ops->list_del_node(file_list, file_node);

	//3.free bit map
	bitmap_clear(fclient->bitmap, fd, 1);

	//4.return code
	return 0;
}

//write data to data_server, only support append now
int fs_append(appendfile_msg_t *appendfile_msg, const char *data)
{
	rpc_client_t *rpc_client = NULL;
	client_append_file_t *client_append_file = NULL;
	int fd;
	opened_file_t *opened_file = NULL;
	list_t *file_list = NULL;
	zclient_t *zclient = NULL;
	pthread_mutex_t *mutex;
	sds lock_name;
	int ret_code = FOK;

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
	client_append_file->source = rpc_client->client_id;

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
		ret_code = FSERVER_ERR;
	}

	//5.copy data from shared memory and send to dataserver
	append_data(fclient, appendfile_msg, rpc_client->recv_buff, data);

	//6.delete lock
	zclient->op->delete_znode(zclient, lock_name, -1);

	//7.copy result to share memory
	if(ret_code != FSERVER_ERR)
	{
		//TODO code transfer
		ret_code = FOK;
	}

	zfree(client_append_file);
	zfree(rpc_client->recv_buff);

	return ret_code;
}

//read data from data_server
int fs_read(readfile_msg_t *readfile_msg, char *data)
{
#if CLIENT_DEBUG
	log_write(LOG_DEBUG, "client server f_read start");
#endif
	rpc_client_t *rpc_client = NULL;
	client_read_file_t *client_read_file = NULL;
	int fd;
	opened_file_t *opened_file = NULL;
	list_t *file_list = NULL;
	zclient_t *zclient = NULL;
	pthread_mutex_t *mutex;
	sds lock_name;
	int ret_code = FOK;

#if CLIENT_DEBUG
	log_write(LOG_DEBUG, "READ A FILE");
#endif

	//1.find right structure
	fd = readfile_msg->fd;
	file_list = fclient->file_list;
	opened_file = file_list->list_ops->list_search_key(file_list, &fd)->value;

	//2.construct client_read_file message
	rpc_client = fclient->rpc_client;
	client_read_file = zmalloc(sizeof(client_read_file_t));
	init_read_msg(client_read_file, readfile_msg, opened_file);
	client_read_file->unique_tag = rpc_client->tag;
	client_read_file->source = rpc_client->client_id;

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
#if CLIENT_DEBUG
	log_write(LOG_DEBUG, "client f_read and offset = %d", client_read_file->offset);
#endif
	//4.send read message to data master
	rpc_client->op->set_send_buff(rpc_client, client_read_file, sizeof(client_read_file_t));
	if(rpc_client->op->execute(rpc_client, COMMAND_WITH_RETURN) < 0)
	{
		ret_code = FSERVER_ERR;
	}

	//5.recv data from dataserver and send to fifo
	read_data(fclient, readfile_msg, rpc_client->recv_buff, data);

#if CLIENT_DEBUG
	log_write(LOG_DEBUG, "client server read data");
#endif

	//6.delete lock
	zclient->op->delete_znode(zclient, lock_name, -1);
#if CLIENT_DEBUG
	log_write(LOG_DEBUG, "client server delete lock");
#endif
	//7.copy result to share memory
	if(ret_code != FSERVER_ERR)
	{
		ret_code = FOK;//TODO code_transfer
	}

#if CLIENT_DEBUG
	log_write(LOG_DEBUG, "client server write to memory");
#endif
	zfree(client_read_file);
	zfree(rpc_client->recv_buff);
#if CLIENT_DEBUG
	log_write(LOG_DEBUG, "client server end f_read");
#endif
	return ret_code;
}

//create file client server and need to know data-master id and tag
fclient_t *create_fclient(int client_id, int target, int tag)
{
	fclient = zmalloc(sizeof(fclient_t));

	if(fclient == NULL)
	{
		return NULL;
	}
	fclient->rpc_client = create_rpc_client(client_id, target, tag);
	fclient->data_master_id = target;

	fclient->zclient = create_zclient(client_id);
	set_zclient(fclient->zclient, target, tag);
	//init bitmap and set bitmap to zero
	fclient->bitmap = (unsigned long *)zmalloc(sizeof(unsigned long) * MAX_FD_NUMBER / BITS_PER_LONG);
	bitmap_empty(fclient->bitmap, MAX_FD_NUMBER);
	//init file list
	fclient->file_list = list_create();
	list_set_free_method(fclient->file_list, free_file);
	list_set_match_method(fclient->file_list, match_file);

	return fclient;
}

void destroy_fclient()
{
	destroy_rpc_client(fclient->rpc_client);

	fclient->zclient->op->stop_zclient(fclient->zclient);
	destroy_zclient(fclient->zclient);

	list_release(fclient->file_list);

	zfree(fclient);
}

void *fclient_run()
{
	zclient_t *zclient = NULL;

	log_write(LOG_INFO, "===client server start run===");
	zclient = fclient->zclient;
	zclient->op->start_zclient(zclient);
	system_test(zclient->client_id);
	return 0;
}
