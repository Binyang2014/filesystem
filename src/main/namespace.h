#ifndef _FILESYSTEM_MAIN_NAMESPACE_H_
#define _FILESYSTEM_MAIN_NAMESPACE_H_

#include <string.h>
#include <stdio.h>
#include "file_name_handle.h"
#include "namespaceparam.h"

#define PARENT_HASH_LENGTH 10
#define CHILD_HASH_LENGTH 10

/*
 * 父目录节点数据结构
 */
struct file_dir_node
{
	char *file_name; 					/*文件名称,包含路径*/
	int *id; 							/*文件所在机器ID列表*/
	int access;							/*文件访问权限*/
	char owner[255]; 					/*文件拥有者*/
	struct file_dir_node *next_dir;		/*下一个目录*/
	struct file_dir_node *next_file;	/*下一个文件*/
	int is_dir; 						/*文件是否是目录*/
	struct file_dir_node **child;		//记录目录下的所有文件,包含目录
	int file_num;						//目录下的文件个数
};

struct file_request_queue{

};

int create_file(char *name, int lenght, int size);

int create_dir(char *name, int length);

int rename_file(char *name, int length);

int rename_dir(char *name, int length);

int del_file(char *name);

int del_dir(char *name);

void list_dir_file(char *name);

void test_create_dir();

#endif
