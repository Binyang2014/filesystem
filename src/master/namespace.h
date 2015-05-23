#ifndef _FILESYSTEM_MAIN_NAMESPACE_H_
#define _FILESYSTEM_MAIN_NAMESPACE_H_

#include <stdio.h>
#include "../structure/basic_queue.h"

/*
 * 父目录节点数据结构
 */
typedef struct file_dir_node
{
	char *file_name; 							/*file full path name*/
	//unsigned char temporary
	unsigned int access;						/*文件访问权限*/
	unsigned int is_dir; 						/*文件是否是目录*/
	unsigned int file_num;						//目录下的文件个数
//	char owner[256]; 							/*文件拥有者*/
	struct file_dir_node *next_dir;				/*下一个目录*/
	struct file_dir_node *next;
	struct file_dir_node *next_file;			/*下一个文件*/
	struct file_dir_node **child;				//记录目录下的所有文件,包含目录
	unsigned long file_size;
	basic_queue_t *location;
}file_dir_node;

typedef struct namespace{
	int parent_hash_length;
	int child_hash_length;
	file_dir_node **parent_dirs;
}namespace;

/*========================Name Space API Declaration=====================*/
namespace *create_namespace(int parent_hash_length, int child_hash_length);
int namespace_create_file(namespace *this, char* name);
int namespace_create_dir(namespace *this, char* name);
int set_file_location(const namespace *namespace, basic_queue_t *location, char *name);
int namespace_rename_file(namespace *this, char* old_name, char *new_name);
int namespace_rename_dir(namespace *this, char* old_name, char *new_name);
int namespace_del_file(namespace *this, char* name);
int namespace_del_dir(namespace *this, char* name);
void namespace_list_dir_file(namespace *this, char* name);

#endif
