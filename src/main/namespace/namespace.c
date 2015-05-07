#include "namespace.h"

/*---------------------private local variables------------------*/
static file_dir_node *par_dirs[PARENT_HASH_LENGTH];

static const char dir_root = '/';
static const char file_separator = '/';

/*---------------------private prototypes-----------------------*/
//verify the path of the file
static int file_path_verify(const char *);
static int dir_path_verify(const char *);
static void path_pre_handle(char *);
static int parse_path(char *, char *, char *, int);

static file_dir_node* find_file_node(const char *);
static file_dir_node* find_dir_node(const char *);
static file_dir_node* new_file_node(const char *);

static void node_free(file_dir_node*);

/*----------------implementation of functions-------------------*/

/*
 * verify the file path
 */
static int file_path_verify(const char *file_path) {
	return 1;
}

/**
 * verify the directory path
 */
static int dir_path_verify(const char *name) {
	return 1;
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
int parse_path(char *parent_dir_name, char *file_name, const char *path, int len) {
	path_pre_handle(path);

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
	strcpy(file_name, file_name_start + 1);
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

static file_dir_node* find_file_node(const char *name) {
	return NULL;
}

static file_dir_node* find_dir_node(const file_dir_node *par_dirs, const char *name) {
	if(par_dirs == NULL || name == NULL)
		return NULL;

	int hash_code = bkdr_hash(name, PARENT_HASH_LENGTH);
	file_dir_node* dir_node = par_dirs[hash_code];
	if(dir_node == NULL)
		return NULL;
	while(dir_node != NULL){
		if(strcmp(dir_node->file_name, name) == 0){
			if(!dir_node->is_dir)
				return NULL;
			return dir_node;
		}
		dir_node = dir_node->next_dir;
	}
	return NULL;
}

static file_dir_node* new_file_node(const char *name) {
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
static int init_root() {
	file_dir_node *root = malloc_dir_node("/");
	if (root == NULL) {
		return -1;
	}

	int root_hash = bkdr_hash("", PARENT_HASH_LENGTH);
	par_dirs[root_hash] = root;
	root = 0;
}

/**
 * Initialize the name space
 * create the root directory "/"
 */
int init() {
	memset(par_dirs, 0, sizeof(struct file_dir_node *) * PARENT_HASH_LENGTH);
	return init_root();
}

/**
 * 创建文件
 * @dir  目录名称
 * @file 文件名称
 * @return 返回创建文件结果码
 */
int create_file(const char * file_path) {
	int length = strlen(file_path);
	char *dir, *file_name, *tmp_path;
	int status = parse_path(dir, file_name, tmp_path, length);
	if (status != 0)
		return status;

	//目录、文件映射码
	int file_hash_code = bkdr_hash(file_name, CHILD_HASH_LENGTH);

	//父目录、父目录文件链节点
	file_dir_node *dir_node = find_dir_node(par_dirs, dir);
	if(dir_node == NULL)
		return DIR_NOT_EXISTS;

	file_dir_node *file_node, *tmp_node;

	tmp_node = dir_node->child + file_hash_code;
	file_node = dir_node->child + file_hash_code;
	while (tmp_node) {
		if (strcmp(tmp_node->file_name, file_name) == 0)
			return FILE_EXISTS;
		tmp_node = tmp_node->next_file;
	}

	struct file_dir_node* new_file = malloc_file_node(file_path);

	if(file_dir_node == NULL)
		return -1;
	return 0;
}

/**
 * @parent_dir 父目录名称
 * @dir		    目录名称
 * 1. 父目录不存在
 * 2. 目录名称不合法(名称格式 长度)
 * 3.
 */
int create_dir(const char *path) {
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
	int path_hash_code = bkdr_hash(tmp_path, PARENT_HASH_LENGTH);
	int dir_hash_code = bkdr_hash(parent_dir, PARENT_HASH_LENGTH);
	int file_hash_code = bkdr_hash(child_dir, CHILD_HASH_LENGTH);

	struct file_dir_node *tmp_node = par_dirs[path_hash_code]; //遍历指针
	struct file_dir_node *parent_node, *child_node;			//目录节点指针 文件节点指针

	//查找目录是否已经存在
	while (1) {
		if (!tmp_node)
			break;
		if (strcmp(tmp_node->file_name, tmp_path) == 0)
			return DIR_EXISTS;
		tmp_node = tmp_node->next_dir;
	}

	tmp_node = par_dirs[dir_hash_code];
	//查找父目录是否存在
	while (tmp_node) {
		if (!tmp_node)
			return DIR_NOT_EXISTS;
		if (strcmp(tmp_node->file_name, parent_dir) == 0)
			break;
		else
			tmp_node = tmp_node->next_dir;
	}
	//父目录节点
	parent_node = tmp_node;
	printf("parent_dir=%s sizeof child=%d\n", parent_dir,
			sizeof parent_node->child);
	child_node = *(parent_node->child + file_hash_code);

	//新的目录节点
	struct file_dir_node *new_dir_node = (struct file_dir_node *) malloc(
			sizeof(struct file_dir_node));
	new_dir_node->file_name = (char *) malloc(sizeof(char) * length); //文件名
	strcpy(new_dir_node->file_name, tmp_path);						//复制文件名
	new_dir_node->file_num = 0;										//文件数量
	new_dir_node->child = (struct file_dir_node **) malloc(
			sizeof(struct file_dir_node*) * CHILD_HASH_LENGTH);
	memset(new_dir_node->child, 0,
			sizeof(struct file_dir_node*) * CHILD_HASH_LENGTH);
	new_dir_node->is_dir = 1;											//是否是目录

	//printf("child_node = %d\n", child_node->next_file);

	new_dir_node->next_dir =
			par_dirs[path_hash_code] ? par_dirs[path_hash_code]->next_dir : 0;
	par_dirs[path_hash_code] = new_dir_node;

	new_dir_node->next_file = child_node ? child_node->next_file : 0;
	*(parent_node->child + file_hash_code) = new_dir_node;

	parent_node->file_num++;

	return CREATE_SUCCESS;
}

int rename_file(const char *old_name, const char *new_name) {
	int new_name_length = strlen(new_name);
	int old_name_length = strlen(old_name);
	char *old_dir, *old_file, *old_path, *new_dir, *new_file, *newpath;

	//int new_status = parse_path()
	return 0;
}

int rename_dir() {
	return 0;
}

int del_dir() {
	return 0;
}

int del_file(char *file_name) {
	return 0;
}

void list_dir_file(char *name) {

}

int test_create_dir() {
	puts("创建目录成功");
	return 0;
}

/**
 * print the file system directory
 */
static void print_dir() {
	int i = 0, j;
	file_dir_node *f;
	file_dir_node *c;
	file_dir_node *cc;
	for (; i < PARENT_HASH_LENGTH; i++) {
		f = par_dirs[i];
		while (f) {
			printf("i = %d dirs = %s |", i, par_dirs[i]->file_name);
			j = 0;
			for (; j < CHILD_HASH_LENGTH; j++) {
				c = *(f->child + j);
				//printf("c====%d ", c);
				while (c) {
					printf("%s ", c->file_name/*, *c->next_file*/);
					c = c->next_file;
				}
			}
			f = f->next_dir;
		}
		if (par_dirs[i])
			puts("");
	}
	f = 0;
	c = 0;
}

