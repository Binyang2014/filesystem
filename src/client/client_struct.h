/*
 * client_struct.h
 *
 * create on: 2015.10.15
 * author: Binyang
 * This file will define some basic structures ahout client and fiiles
 */

#ifndef CLIENT_STRUCT_H_
#define CLIENT_STRUCT_H_

#include "global.h"
#include "sds.h"
#include "basic_list.h"

#define MAX_FILE_PATH 255
#define FOK 0
#define FNO_EXISIT 1
#define FSERVER_ERR 2
#define FNO_PERMISSION 3
#define FPATH_TOO_LONG 4

#define FCREATE_OP 0001
#define FOPEN_OP 0002

//==============ABOUT FILE LIST STRUCTURE=============
struct data_node
{
	int data_server_num;
	size_t data_len;
};

struct file_info
{
	sds file_path;
	size_t file_len;
	list_t *data_nodes;
	size_t file_offset;
};

struct opened_file
{
	struct file_info f_info;
	int position;
	int fd;
	open_mode_t open_mode;
	int version;
};

//======================ABOUT FILE RETURE MESSAGE=================
struct file_ret_msg
{
	int ret_code;
	int fd;
};

//====================ABOUT FILE OPERATION MESSAGE================
struct openfile_msg
{
	uint16_t operation_code;
	char file_path[MAX_FILE_PATH + 1];
	open_mode_t open_mode;
};

struct createfile_msg
{
	uint16_t operation_code;
	char file_path[MAX_FILE_PATH + 1];
	open_mode_t open_mode;
	f_mode_t mode;
};

typedef struct data_node data_node_t;
typedef struct file_info file_info_t;
typedef struct opened_file opened_file_t;
typedef struct createfile_msg create_msg_t;
typedef struct openfile_msg openfile_msg_t;
typedef struct file_ret_msg file_ret_msg_t;

opened_file_t *create_file(const char *file_path, int position, open_mode_t
		open_mode);
void free_file(opened_file_t *opened_file);
#endif
