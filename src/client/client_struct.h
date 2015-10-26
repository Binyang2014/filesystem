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
#define FAPPEND_OP 0003
#define FREAD_OP 0004
#define FLCOSE_OP 0005
#define FREMOVE_OP 0006
#define FSTOP_OP 0007

//==============ABOUT FILE LIST STRUCTURE=============
struct data_node
{
	int data_server_id;
	int chunks_num;
	uint64_t *chunks_id;
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

struct appendfile_msg
{
	uint16_t operation_code;
	size_t data_len;
	int fd;
};

struct readfile_msg
{
	uint16_t operation_code;
	size_t offset;
	size_t data_len;
	int fd;
};

struct removefile_msg
{
	uint16_t operation_code;
	char file_path[MAX_FILE_PATH + 1];
};

struct closefile_msg
{
	uint16_t operation_code;
	int fd;
};

struct stopclient_msg
{
	uint16_t operation_code;
};

union file_msg
{
	struct createfile_msg;
	struct openfile_msg;
	struct readfile_msg;
	struct appendfile_msg;
	struct removefile_msg;
	struct closefile_msg;
	struct stopclient_msg;
};

typedef struct data_node data_node_t;
typedef struct file_info file_info_t;
typedef struct opened_file opened_file_t;
typedef struct createfile_msg create_msg_t;
typedef struct openfile_msg openfile_msg_t;
typedef struct file_ret_msg file_ret_msg_t;
typedef struct appenfile_msg appendfile_msg_t;
typedef struct readfile_msg readfile_msg_t;
typedef struct removefile_msg removefile_msg_t;
typedef struct stopclient_msg stopclient_msg_t;
typedef union file_msg file_msg_t;

opened_file_t *create_file(const char *file_path, int position, open_mode_t
		open_mode);
void free_file(opened_file_t *opened_file);
int match_file(void *ptr, void *key);
#endif
