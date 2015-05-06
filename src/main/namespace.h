#ifndef _FILESYSTEM_MAIN_NAMESPACE_H_
#define _FILESYSTEM_MAIN_NAMESPACE_H_

#include <string.h>
#include <stdio.h>
#include "namespace_param.h"
#include "data_server_location.h"

#define PARENT_HASH_LENGTH 1024
#define CHILD_HASH_LENGTH 1024

/**
 * 校验文件路径
 */
int file_path_verify(const char *file_path);

/**
 * 校验目录路径
 */
int dir_path_verify();

void path_pre_handle(char *path);

/**
 * 将path分为目录名 + 文件名
 * @parent_dir_name 保存目录名称
 * @file_name 保存文件名称
 */
static int parse_path(char *parent_dir_name, char *file_name, char *path, int len);

/*
 * 父目录节点数据结构
 */
typedef struct file_dir_node
{
	char *file_name; 					/*file full path name*/
	unsigned int *id; 							/*文件所在机器ID列表*/
	unsigned int access;							/*文件访问权限*/
	unsigned int is_dir; 						/*文件是否是目录*/
	unsigned int file_num;						//目录下的文件个数
//	char owner[256]; 					/*文件拥有者*/
	struct file_dir_node *next_dir;			/*下一个目录*/
	struct file_dir_node *next_file;			/*下一个文件*/
	struct file_dir_node **child;				//记录目录下的所有文件,包含目录
	unsigned long file_size;

}file_dir_node;

static int create_file(char *name, int length, int size);

static int create_dir(char *name, int length);

static int rename_file(char *name, int length);

static int rename_dir(char *name, int length);

static int del_file(char *name);

static int del_dir(char *name);

static void list_dir_file(char *name);

static void test_create_dir();

#endif
