#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <regex.h>
#include "namespace.h"
#include "../structure/namespace_code.h"
#include "../tool/hash.h"
#include "../tool/errinfo.h"


/*---------------------private local variables------------------*/
static const char dir_root = '/';
static const char file_separator = '/';
static const char *filesystem_root_name = "/";
static const char *file_path_pattern = "^(/[0-9a-zA-Z]+)+$";
static const int file_name_size = 1 << 8;

/*---------------------private prototypes-----------------------*/
//verify the path of the file
static int path_verify(const char *);
static void path_pre_handle(char *);
static int parse_path(char *, const char *, int);
static int in_same_dir(const char *file_name_a, const char *file_name_b);

static file_dir_node* find_file_node(const namespace *this, const char *name);
static file_dir_node* find_dir_node(const namespace *this, const char *name);

static void node_free(file_dir_node*);

/*----------------implementation of functions-------------------*/

typedef struct file_path_regex{
	regex_t reg;
	regmatch_t pm;
}file_path_regex;

//TODO need to free the singleTon regex
static file_path_regex *get_regex(){
	static file_path_regex *regex;
	if(regex == NULL){
		puts("regex is NULL");
		regex = (file_path_regex *)malloc(sizeof(file_path_regex));
		if(regex != NULL){
			memset(regex, 0 , sizeof(file_path_regex));
			regcomp(&(regex->reg), file_path_pattern, REG_EXTENDED);
		}
	}
	return regex;
}

/*
 * verify the file path
 */
static int path_verify(const char *path) {
	int length = strlen(path);
	if(length > file_name_size){
		err_ret("path %s length is too long", path);
		return ILLEGAL_FILE_PATH;
	}

	file_path_regex *regex = get_regex();
	if(regex == NULL){
		err_ret("failed to malloc enough space for the regex of verify file path");
		return NO_ENOUGH_SPACE;
	}

	return regexec(&(regex->reg), path, 1, &(regex->pm), 0) == REG_NOMATCH ? REG_NOMATCH : OPERATE_SECCESS;
}

/**
 * handle the path
 * exchange character '/' into char * end signal 0
 */
static void path_pre_handle(char *path) {
	int length = strlen(path);
	if (length == 0)
		return;

	length--;
	while (length >= 1 && *(path + length) == dir_root) {
		*(path + length) = 0;
		length--;
	}

}

/**
 * parse the path into directory name and file name
 * @parent_dir_name directory name
 * @file_name file name
 */
static int parse_path(char *parent_dir_name, const char *path, int len) {
	char *tmp_path = (char*) malloc(sizeof(char) * len);
	if (tmp_path == NULL)
		return NO_ENOUGH_SPACE;
	strcpy(tmp_path, path);

	/*separator the path and copy the file name into variable file_name*/
	char *file_name_start = strrchr(tmp_path, file_separator);
	if(file_name_start == tmp_path)
		*(file_name_start + 1) = 0;
	else
		*file_name_start = 0;

	strcpy(parent_dir_name, tmp_path);

	free(tmp_path);
	tmp_path = NULL;
	return OPERATE_SECCESS;
}

static int in_same_dir(const char *file_name_a, const char *file_name_b){
	int new_name_length = strlen(file_name_a);
	int old_name_length = strlen(file_name_b);
	char old_dir[file_name_size], new_dir[file_name_size];
	parse_path(old_dir, file_name_a, old_name_length);
	parse_path(new_dir, file_name_b, new_name_length);
	/*only in the same directory, file can be renamed*/
	return strcmp(old_dir, new_dir);
}

static file_dir_node* malloc_node(const char *name, int is_dir, int child_hash_length) {
	file_dir_node *root = (struct file_dir_node *) malloc(sizeof(struct file_dir_node));
	if(root == NULL)
		return NULL;

	root->file_name = (char *) malloc(sizeof(char) * file_name_size);
	if (root->file_name == NULL) {
		free(root);
		return NULL;
	}
	strcpy(root->file_name, name);

	if(is_dir){
		root->child = (file_dir_node **) malloc(sizeof(file_dir_node *) * child_hash_length);
		if(root->child == NULL){
			node_free(root);
			err_ret("There is no enough space");
			return NULL;
		}
		memset(root->child, 0, sizeof(file_dir_node *) * child_hash_length);
	}

	root->file_num = 0;
	root->next_file = 0;
	root->next_dir = 0;
	root->is_dir = is_dir;
	return root;
}

static file_dir_node* malloc_file_node(const char *file_name, int child_hash_length) {
	return malloc_node(file_name, 0, child_hash_length);
}

static file_dir_node* malloc_dir_node(const char *dir_name, int child_hash_length) {
	return malloc_node(dir_name, 1, child_hash_length);
}

static file_dir_node* find_file_node(const namespace *this, const char *name) {
	assert(this != NULL);

	char dir_name[file_name_size];
	parse_path(dir_name, name, strlen(name));
	int dir_hash = bkdr_hash(dir_name, this->parent_hash_length);
	int chi_hash = bkdr_hash(name, this->child_hash_length);
	//file_dir_node *par_dirs = this->parent_dirs[dir_hash];

	file_dir_node *par_node = find_dir_node(this, dir_name);
	if(par_node == NULL)
		return NULL;

	file_dir_node *chi_node = par_node->child[chi_hash];
	while(chi_node != NULL){
		if(strcmp(chi_node->file_name, name) == 0){
//			if(chi_node->is_dir)
//				return NULL;
			return chi_node;
		}
		chi_node = chi_node->next_file;
	}
	return NULL;
}

static file_dir_node* find_dir_node(const namespace *this, const char *name) {
	assert(this != NULL);

	int hash_code = bkdr_hash(name, this->parent_hash_length);
	file_dir_node* dir_node = *(this->parent_dirs + hash_code);

	while(dir_node != NULL){
		if(strcmp(dir_node->file_name, name) == 0){
			return dir_node;
		}
		dir_node = dir_node->next_dir;
	}
	return NULL;
}

static void node_free(file_dir_node* node){
	if(node != NULL){
		free(node->file_name);
		if(node->is_dir)
			free(node->child);
		free(node);
		node = NULL;
	}
}

/**
 * initialize the root of name space
 * 1. allocate the space for first level mapping
 * 2.
 */
static int init_root(namespace *this) {
	file_dir_node *root = malloc_dir_node(filesystem_root_name, this->child_hash_length);
	if (root == NULL) {
		return NO_ENOUGH_SPACE;
	}

	int root_hash = bkdr_hash(filesystem_root_name, this->parent_hash_length);
	*(this->parent_dirs + root_hash) = root;
	root = 0;
	return OPERATE_SECCESS;
}

/**
 * Initialize the name space
 * create the root directory "/"
 */
static int init(namespace *this) {
	memset(this->parent_dirs, 0, sizeof(struct file_dir_node *) * this->parent_hash_length);
	return init_root(this);
}

namespace *create_namespace(int parent_hash_length, int child_hash_length){
	err_ret("namespace.c start create name space");
	namespace *this = (namespace *)malloc(sizeof(namespace));

	if(this == NULL){
		return NULL;
	}

	this->parent_dirs = (file_dir_node**)malloc(sizeof(file_dir_node*) * parent_hash_length);
	if(this->parent_dirs == NULL){
		free(this);
		err_sys("namespace.c failed to allocate directory space");
		return NULL;
	}

	this->parent_hash_length = parent_hash_length;
	this->child_hash_length = child_hash_length;

	if(init(this) != OPERATE_SECCESS){
		err_ret("namespace.c create name space SUCCESS");
		return NULL;
	}
	err_ret("namespace.c: create name space success");
	return this;
}

/**
 * In order to create a file, we must ensure the file doesn't exist and parent directory must exist
 * 1. check if the parent directory exists
 * 2. check if the file exists
 * 3. create the file or return error code
 * @this  name space
 * @file_path file path
 * @return result code
 */
int namespace_create_file(namespace *this, char * file_path) {
	path_pre_handle(file_path);
	if(path_verify(file_path) != OPERATE_SECCESS){
		return ILLEGAL_FILE_PATH;
	}
	int length = strlen(file_path);
	char dir[file_name_size];
	char tmp_path[file_name_size];
	strcpy(tmp_path, file_path);
	int status = parse_path(dir, tmp_path, length);
	if (status != OPERATE_SECCESS)
		return status;

	//parent directory hash code
	int file_hash_code = bkdr_hash(file_path, this->child_hash_length);

	//parent directory node
	//printf("dir name = %s\n", dir);
	file_dir_node *dir_node = find_dir_node(this, dir);
	if(dir_node == NULL)
		return DIR_NOT_EXISTS;

	file_dir_node *file_node = find_file_node(this, file_path);

	if(file_node != NULL){
		err_ret("exists");
		return FILE_EXISTS;
	}

	struct file_dir_node* new_file = malloc_file_node(file_path, this->child_hash_length);
	if(new_file == NULL)
		return NO_ENOUGH_SPACE;

	new_file->next_file = *(dir_node->child + file_hash_code);
	*(dir_node->child + file_hash_code) = new_file;

	dir_node->file_num++;

	return OPERATE_SECCESS;
}

/**
 * @path
 * 1. parent directory may not exist
 * 2. illegal directory name
 * 3.
 */
int namespace_create_dir(namespace *this, char *path) {
	path_pre_handle(path);
	if(path_verify(path) != OPERATE_SECCESS)
		return ILLEGAL_FILE_PATH;

	int length = strlen(path) + 1;
	char *parent_dir = (char *) malloc(length);

	if(parent_dir == NULL){
		err_ret("no enough memory when malloc space for creating a new dir %s", path);
		return NO_ENOUGH_SPACE;
	}

	//err_msg(" creating dir -> before parse_path");
	int status = parse_path(parent_dir, path, length);
	if (status == ILLEGAL_PATH) {
		return ILLEGAL_PATH;
	}

	//目录 文件 路径的hash映射
	int path_hash_code = bkdr_hash(path, this->parent_hash_length);
	int dir_hash_code = bkdr_hash(parent_dir, this->parent_hash_length);
	//int file_hash_code = bkdr_hash(child_dir, this->child_hash_length);

	//file_dir_node *tmp_node = this->parent_dirs[path_hash_code]; //遍历指针

	//err_msg(" creating dir -> before find dir_node");
	//parent directory node
	file_dir_node *parent_node = find_dir_node(this, parent_dir);
	if(parent_node == NULL){
		err_msg(" creating dir -> directory %s not exists", parent_dir);
		return DIR_NOT_EXISTS;
	}

	//err_msg(" creating dir -> find dir_node");
	//child node
	file_dir_node *child_node = find_dir_node(this, path);
	if(child_node != NULL){
		return DIR_EXISTS;
	}

	//err_msg(" creating dir -> new dir_node");
	//新的目录节点
	file_dir_node *new_dir_node = malloc_dir_node(path, this->child_hash_length);

	if(new_dir_node == NULL){
		err_ret("No enough space for create dir");
	}

	new_dir_node->next_dir = *(this->parent_dirs + path_hash_code);
	*(this->parent_dirs + path_hash_code) = new_dir_node;

	path_hash_code = bkdr_hash(path, this->child_hash_length);
	new_dir_node->next_file = *(parent_node->child + path_hash_code);
	*(parent_node->child + path_hash_code) = new_dir_node;

	parent_node->file_num++;

	//err_msg("creating dir -> OK");
	free(parent_dir);
	parent_dir = NULL;
	return CREATE_SUCCESS;
}

int set_file_location(const namespace *namespace, basic_queue_t *file_location, char *name){
	file_dir_node *node = find_file_node(namespace, name);
	if(node == NULL){
		return FILE_NOT_EXISTS;
	}

	node->location = file_location;
	return OPERATE_SECCESS;
}

/**
 * since the name space is implemented by full path name, a dir name modify should cause it's children's
 * name modify, it may cost much right now and it will be implemented next
 */
int namespace_rename_file(namespace *this, char *old_name, char *new_name) {
	path_pre_handle(old_name);
	path_pre_handle(new_name);

	//TODO need to check whether the two names are in the same directory
	if(path_verify(old_name) != OPERATE_SECCESS || path_verify(new_name) != OPERATE_SECCESS){
		return ILLEGAL_FILE_PATH;
	}

	/*only in the same directory, file can be renamed*/
	if(in_same_dir(old_name, new_name) != 0)
		return NOT_THE_SAME_DIR;

	file_dir_node * node = find_file_node(this, old_name);
	if(node == NULL){
		return FILE_NOT_EXISTS;
	}

	int new_name_length = strlen(new_name);
	char *name = (char *)malloc(new_name_length);
	if(name == NULL){
		return -1;
	}

	free(node->file_name);
	node->file_name = name;
	return 0;
}

int namespace_rename_dir(namespace *this, char *old_name, char *new_name) {
	path_pre_handle(old_name);
	path_pre_handle(new_name);

	if(path_verify(old_name) != OPERATE_SECCESS || path_verify(new_name) != OPERATE_SECCESS){
		return ILLEGAL_FILE_PATH;
	}

	int new_len = strlen(new_name);
	/*only in the same directory, file can be renamed*/
	if(in_same_dir(old_name, new_name) != 0)
		return NOT_THE_SAME_DIR;

	file_dir_node *dir_node = find_dir_node(this, old_name);
	if(dir_node == NULL){
		return DIR_NOT_EXISTS;
	}
	//TODO need a right code
	char *name = (char *)malloc(new_len);
	if(name == NULL)
		return -1;

	free(dir_node->file_name);
	dir_node->file_name = name;
	name = NULL;
	return OPERATE_SECCESS;
}

int namespace_del_file(namespace *this, char* name){
	return 0;
}

int namespace_del_dir(namespace *this, char* name){
	return 0;
}

void namespace_list_dir_file(namespace *this, char* name){

}


