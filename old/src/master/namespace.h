#ifndef _FILESYSTEM_MAIN_NAMESPACE_H_
#define _FILESYSTEM_MAIN_NAMESPACE_H_

#include <stdio.h>
#include "../structure/basic_queue.h"

/*
 * structure of file and directory node
 */
typedef struct file_dir_node
{
	char *file_name; 							/*file full path name*/
	unsigned char temporary;
	unsigned int access;						/*file access mode*/
	unsigned int is_dir; 						/*whether the file is directory*/
	unsigned int file_num;						/*number of sub file*/
	//char owner[256]; 							/*file owner*/
	struct file_dir_node *next_dir;				/*next directory*/
	struct file_dir_node *next_file;			/*next file*/
	struct file_dir_node **child;				/*children file*/
	unsigned long file_size;					/*file size*/
	basic_queue_t *location;					/*file location*/
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
basic_queue_t *get_file_location(const namespace *namespace, char *name);
int namespace_rename_file(namespace *this, char* old_name, char *new_name);
int namespace_rename_dir(namespace *this, char* old_name, char *new_name);
int namespace_del_file(namespace *this, char* name);
int namespace_del_dir(namespace *this, char* name);
void namespace_list_dir_file(namespace *this, char* name);

#endif
