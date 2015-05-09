#include "namespace.h"
#include "../structure/namespace_code.h"

/*---------------------private local variables------------------*/
static const char dir_root = '/';
static const char file_separator = '/';
static const int file_name_size = 256;

/*---------------------private prototypes-----------------------*/
//verify the path of the file
static int path_verify(const char *);
static void path_pre_handle(char *);
static int parse_path(char *, char *, char *, int);

static file_dir_node* find_file_node(const char *);
static file_dir_node* find_dir_node(const char *);

static void node_free(file_dir_node*);

/*----------------implementation of functions-------------------*/

/*
 * verify the file path
 */
static int path_verify(const char *path) {
	return OPERATE_SECCESS;
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
	if (!file_path_verify(path))
		return ILLEGAL_FILE_PATH;

	char *tmp_path = (char*) malloc(sizeof(char) * len);
	if (tmp_path == NULL) {
		return -1;
	} else {
		strcpy(tmp_path, path);
	}

	/*separator the path and copy the file name into variable file_name*/
	char *file_name_start = strrchr(tmp_path, file_separator);
	*file_name_start = 0;
	strcpy(parent_dir_name, tmp_path);

	free(tmp_path);
	tmp_path = NULL;
	return 0;
}

static file_dir_node* malloc_node(const char *name, int is_dir) {
	file_dir_node *root = (struct file_dir_node *) malloc(sizeof(struct file_dir_node));
	if(root == NULL)
		return NULL;

	root->file_name = (char *) malloc(sizeof(char) * 256);
	if (root->file_name == NULL) {
		free(root);
		return NULL;
	}
	strcpy(root->file_name, name);

	if(is_dir){
		root->child = (struct file_dir_node *) malloc(sizeof(struct file_dir_node *) * CHILD_HASH_LENGTH);
		if(root->child == NULL)
			node_free(root);
	}

	root->file_num = 0;
	root->next_file = 0;
	root->next_dir = 0;
	root->is_dir = is_dir;
	return root;
}

static file_dir_node* malloc_file_node(const char *file_name) {
	return malloc_node(file_name, 0);
}

static file_dir_node* malloc_dir_node(const char *dir_name) {
	return malloc_node(dir_name, 1);
}

static file_dir_node* find_file_node(namespace *this, const char *dir_name, const char *name) {
	assert(this != NULL);

	int dir_hash = bkdr_hash(dir_name, this->parent_hash_length);
	int chi_hash = bkdr_hash(name, this->child_hash_length);
	//file_dir_node *par_dirs = this->parent_dirs[dir_hash];

	file_dir_node *par_node = find_dir_node(this, dir_name);
	if(par_node == NULL)
		return NULL;

	file_dir_node *chi_node = par_node->child[chi_hash];
	while(chi_node != NULL){
		if(strcmp(chi_node->file_name, name) == 0){
			if(chi_node->is_dir)
				return NULL;
			return chi_node;
		}
		chi_node = chi_node->next_file;
	}
	return NULL;
}

static file_dir_node* find_dir_node(namespace *this, const char *name) {
	assert(this != NULL);

	int hash_code = bkdr_hash(name, this->parent_hash_length);
	file_dir_node* dir_node = this->parent_dirs[hash_code];
	if(dir_node == NULL)
		return NULL;

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
	file_dir_node *root = malloc_dir_node("/");
	if (root == NULL) {
		return -1;
	}

	int root_hash = bkdr_hash("", this->parent_hash_length);
	this->parent_dirs[root_hash] = root;
	root = 0;
}

/**
 * Initialize the name space
 * create the root directory "/"
 */
static int init(namespace *this) {
	memset(this->parent_dirs, 0, sizeof(struct file_dir_node *) * this->parent_hash_length);
	return init_root();
}

namespace *create_namespace(int parent_hash_length, int child_hash_length){
	namespace *this = (namespace *)malloc(sizeof(namespace));

	if(this == NULL){
		return NULL;
	}

	this->parent_dirs = (file_dir_node*)malloc(sizeof(file_dir_node*) * parent_hash_length);
	if(this->parent_dirs == NULL){
		free(this);
		//TODO log
		return NULL;
	}

	this->parent_hash_length = parent_hash_length;
	this->child_hash_length = child_hash_length;
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
int namespace_create_file(namespace *this, const char * file_path) {
	path_pre_handle(file_path);
	int length = strlen(file_path);
	char dir[file_name_size];
	char tmp_path[file_name_size];
	strcpy(tmp_path, file_path);
	int status = parse_path(dir, tmp_path, length);
	if (status != 0)
		return status;

	//parent directory hash code
	int file_hash_code = bkdr_hash(file_path, this->child_hash_length);

	//parent directory node
	file_dir_node *dir_node = find_dir_node(this->parent_dirs, dir);
	if(dir_node == NULL)
		return DIR_NOT_EXISTS;

	file_dir_node *file_node = find_file_node(file_path);
	if(file_node != NULL)
		return FILE_EXISTS;

	struct file_dir_node* new_file = malloc_file_node(file_path);
	if(file_dir_node == NULL)
		return -1;

	new_file->next_file = *(dir_node->child + file_hash_code);
	*(dir_node->child + file_hash_code) = new_file;

	return 0;
}

/**
 * @path
 * 1. parent directory may not exist
 * 2. illegal directory name
 * 3.
 */
int namespace_create_dir(namespace *this, const char *path) {
	int length = strlen(path) + 1;
	char *parent_dir = (char *) malloc(length);
	char *child_dir = (char *) malloc(length);
	char *tmp_path = (char *) malloc(length);
	strcpy(tmp_path, path);

	int status = parse_path(parent_dir, child_dir, tmp_path, length);
	if (status == ILLEGAL_PATH) {
		return ILLEGAL_PATH;
	}

	//目录 文件 路径的hash映射
	int path_hash_code = bkdr_hash(tmp_path, this->parent_hash_length);
	int dir_hash_code = bkdr_hash(parent_dir, this->parent_hash_length);
	int file_hash_code = bkdr_hash(child_dir, this->child_hash_length);

	//file_dir_node *tmp_node = this->parent_dirs[path_hash_code]; //遍历指针
	file_dir_node *parent_node;			//目录节点指针 文件节点指针

	//parent directory node
	file_dir_node *parent_node = find_dir_node(this->parent_dirs, parent_dir);
	if(parent_node == NULL){
		return DIR_NOT_EXISTS;
	}

	//child node
	file_dir_node *child_node = find_dir_node(this->parent_dirs, parent_dir);
	if(child_node != NULL){
		return DIR_EXISTS;
	}

	//新的目录节点
	file_dir_node *new_dir_node = malloc_dir_node(tmp_path);

	new_dir_node->next_dir = this->parent_dirs[path_hash_code];
	this->parent_dirs[path_hash_code] = new_dir_node;

	new_dir_node->next_file = child_node ? child_node->next_file : 0;
	*(parent_node->child + file_hash_code) = new_dir_node;

	parent_node->file_num++;

	return CREATE_SUCCESS;
}

int namespace_rename_file(namespace *this, const char *old_name, const char *new_name) {
	int new_name_length = strlen(new_name);
	int old_name_length = strlen(old_name);

	//TODO need to check whether the two names are in the same directory

	file_dir_node * node = find_file_node(this, old_name);
	if(node == NULL){
		return FILE_NOT_EXISTS;
	}

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

	int old_len = strlen(old_name);
	int new_len = strlen(new_name);
	return 0;
}

int namespace_del_dir() {
	return 0;
}

int namespace_del_file(char *file_name) {
	return 0;
}

void namespace_list_dir_file(char *name) {

}


