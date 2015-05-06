#ifndef _FILESYSTEM_MAIN_NAMESPACE_H_
#define _FILESYSTEM_MAIN_NAMESPACE_H_

#include <string.h>
#include <stdio.h>
#include "data_server_location.h"
#include "namespace_code.h"

#define PARENT_HASH_LENGTH 1024
#define CHILD_HASH_LENGTH 1024

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

int namespace_create_file(char *name, int length, int size);

int namespace_create_dir(char *name, int length);

int namespace_rename_file(char *name, int length);

int namespace_rename_dir(char *name, int length);

int namespace_del_file(char *name);

int namespace_del_dir(char *name);

void namespace_list_dir_file(char *name);

static void test_create_dir();

#endif
