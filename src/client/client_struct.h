/*
 * client_struct.h
 *
 * create on: 2015.10.15
 * author: Binyang
 * This file will define some basic structures ahout client and fiiles
 */

#ifndef CLIENT_STRUCT_H_
#define CLIENT_STRUCT_H_

#include "sds.h"
#include "basic_list.h"

enum file_stat
{
	F_RDONLY,
	F_WRONLY,
	F_RDWR,
	F_APPEND
};

struct data_node
{
	int data_server_num;
	size_t data_len;
};

struct file_info
{
	sds file_path;
	size_t file_len;
	list_t data_nodes;
};

struct opened_file
{
	struct file_info f_info;
	int position;
	enum file_stat f_stat;
};

typedef enum file_stat file_stat_t;
typedef struct data_node data_node_t;
typedef struct file_info file_info_t;
typedef struct opened_file opened_file_t;

#endif
