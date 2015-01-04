
#include "namespace.h"

struct file_dir_node *par_dirs[PARENT_HASH_LENGTH];

void print_dir(){
	int i = 0, j;
	struct file_dir_node *f;
	struct file_dir_node *c;
	struct file_dir_node *cc;
	for (; i < PARENT_HASH_LENGTH; i++)
	{
		f =  par_dirs[i];
		while (f){
			printf("i = %d dirs = %s |", i, par_dirs[i]->file_name);
			j = 0;
			for(; j < CHILD_HASH_LENGTH; j++){
				c = *(f->child + j);
				//printf("c====%d ", c);
				while(c){
					printf("%s ", c->file_name/*, *c->next_file*/);
					c = c->next_file;
				}
			}
			f = f->next_dir;
		}
		if(par_dirs[i])
			puts("");
	}
	f = 0;
	c = 0;
}

void init(){
	memset(par_dirs, 0, sizeof(struct file_dir_node *) * PARENT_HASH_LENGTH);
	struct file_dir_node *root = (struct file_dir_node *)malloc(sizeof(struct file_dir_node));
	root->file_name = (char *)malloc(sizeof(char) * 256);
	strcpy(root->file_name, "");
	root->file_num = 0;
	root->next_file = 0;
	root->next_dir = 0;
	root->is_dir=1;
	root->child = (struct file_dir_node **)malloc(sizeof(struct file_dir_node *) * CHILD_HASH_LENGTH);
	memset(root->child, 0, sizeof(struct file_dir_node*) * CHILD_HASH_LENGTH);
	int root_hash = bkdr_hash("", PARENT_HASH_LENGTH);
	par_dirs[root_hash] = root;
	root = 0;
}

/**
 * 创建文件
 * @dir  目录名称
 * @file 文件名称
 * @return 返回创建文件结果码
 */
int create_file(const char * file_path)
{
	int length = strlen(file_path);
	char *dir, *file_name, *tmp_path;
	int status = parse_path(dir, file_name, tmp_path, length);
	if(status == illegal_path)
		return illegal_path;

	//目录、文件映射码
	int dir_hash_code = bkdr_hash(dir, PARENT_HASH_LENGTH);
	int file_hash_code = bkdr_hash(file_name, CHILD_HASH_LENGTH);

	//父目录、父目录文件链节点
	struct file_dir_node *dir_node, *file_node, *tmp_node;
	tmp_node = par_dirs[dir_hash_code];

	//查找目录节点
	while(1){
		if(!tmp_node)
			return dir_not_exists;
		if(strcmp(tmp_node->file_name, dir) == 0)
			break;
		tmp_node = tmp_node->next_dir;
	}
	dir_node = tmp_node;

	tmp_node = *(dir_node->child + file_hash_code);
	file_node = *(dir_node->child + file_hash_code);
	while(tmp_node){
		if(strcmp(tmp_node->file_name, file_name) == 0)
			return file_exists;
		tmp_node = tmp_node->next_file;
	}

	struct file_dir_node* new_file = (struct file_dir_node *)malloc(sizeof(struct file_dir_node));
	new_file->file_name = (char *)malloc(sizeof(char) * length);
	strcpy(new_file->file_name, tmp_path);
	new_file->child = 0;
	new_file->file_num = 0;
	new_file->is_dir = 0;
	new_file->next_dir = 0;

	return 0;
}

/**
 * @parent_dir 父目录名称
 * @dir		    目录名称
 * 1. 父目录不存在
 * 2. 目录名称不合法(名称格式 长度)
 * 3.
 */
int create_dir(const char *path)
{
	int length = strlen(path) + 1;
	char *parent_dir = (char *)malloc(sizeof(char) * length);
	char *child_dir = (char *)malloc(sizeof(char) * length);
	char *tmp_path = (char *)malloc(sizeof(char) * length);
	strcpy(tmp_path, path);

	int status = parse_path(parent_dir, child_dir, tmp_path, length);
	if(status == illegal_path){
		return illegal_path;
	}

	//目录 文件 路径的hash映射
	int path_hash_code = bkdr_hash(tmp_path, PARENT_HASH_LENGTH);
	int dir_hash_code = bkdr_hash(parent_dir, PARENT_HASH_LENGTH);
	int file_hash_code = bkdr_hash(child_dir, CHILD_HASH_LENGTH);

	struct file_dir_node *tmp_node = par_dirs[path_hash_code]; //遍历指针
	struct file_dir_node *parent_node, *child_node;			//目录节点指针 文件节点指针

	//查找目录是否已经存在
	while(1)
	{
		if(!tmp_node)
			break;
		if(strcmp(tmp_node->file_name, tmp_path) == 0)
			return dir_exists;
		tmp_node = tmp_node->next_dir;
	}

	tmp_node = par_dirs[dir_hash_code];
	//查找父目录是否存在
	while(tmp_node)
	{
		if(!tmp_node)
			return dir_not_exists;
		if(strcmp(tmp_node->file_name, parent_dir) == 0)
			break;
		else
			tmp_node = tmp_node->next_dir;
	}
	//父目录节点
	parent_node = tmp_node;
	printf("parent_dir=%s sizeof child=%d\n",  parent_dir, sizeof parent_node->child);
	child_node = *(parent_node->child + file_hash_code);

	//新的目录节点
	struct file_dir_node *new_dir_node = (struct file_dir_node *)malloc(sizeof(struct file_dir_node));
	new_dir_node->file_name = (char *)malloc(sizeof(char) * length); //文件名
	strcpy(new_dir_node->file_name, tmp_path);						//复制文件名
	new_dir_node->file_num = 0;										//文件数量
	new_dir_node->child=(struct file_dir_node **)malloc(sizeof(struct file_dir_node*) * CHILD_HASH_LENGTH);
	memset(new_dir_node->child, 0, sizeof(struct file_dir_node*) * CHILD_HASH_LENGTH);
	new_dir_node->is_dir=1;											//是否是目录

	//printf("child_node = %d\n", child_node->next_file);

	new_dir_node->next_dir = par_dirs[path_hash_code] ? par_dirs[path_hash_code]->next_dir : 0;
	par_dirs[path_hash_code] = new_dir_node;

	new_dir_node->next_file= child_node ? child_node->next_file : 0;
	*(parent_node->child + file_hash_code) = new_dir_node;

	parent_node->file_num++;

	return create_success;
}

int rename_file(const char *old_name, const char *new_name)
{
	int new_name_length = strlen(new_name);
	int old_name_length = strlen(old_name);
	char *old_dir, *old_file, *old_path, *new_dir, *new_file, *newpath;

	//int new_status = parse_path()
	return 0;
}

int rename_dir()
{
	return 0;
}

int del_dir()
{
	return 0;
}

int del_file(char *file_name){
	return 0;
}

void list_dir_file(char *name){

}

int test_create_dir(){
	puts("创建目录成功");
	return 0;
}

//int main(){
//	init();
//	printf("创建文件返回=%d\n", create_dir("/123"));
//	printf("创建文件返回=%d\n", create_dir("/123/34"));
//	print_dir();
//	return 0;
//}
