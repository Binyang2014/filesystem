#include <string.h>
#include <stdio.h>
#include "file_name_handle.h"
#include "../constant/namespacestatus.h"
#include "../constant/namespaceparam.h"
#include "../tool/hash.h"

#define PARENT_HASH_LENGTH 10
#define CHILD_HASH_LENGTH 10
/*
 * 父目录节点数据结构
 */
typedef struct FileDirNode{
	char file_name[255]; 		/*文件名称*/
	int *id; 					/*文件所在机器ID列表*/
	int access;					/*文件访问权限*/
	char owner[255]; 			/*文件拥有者*/
	struct FileDirNode *next;
	int is_dir; 				/*文件是否是目录*/
	struct FileDirNode *child;
	int file_num;				//目录下的文件个数
}FileDirNode;

/**
 *
 */
struct FileDirNode *par_dirs[PARENT_HASH_LENGTH];


void print_dir(){
	int i = 0;
	for (; i < PARENT_HASH_LENGTH; i++)
	{
		if (par_dirs[i])
			puts(par_dirs[i]->file_name);
	}
}

void init(){
	memset(par_dirs, 0, sizeof(struct FileDirNode *) * PARENT_HASH_LENGTH);
	struct FileDirNode *root = (struct FileDirNode *)malloc(sizeof(struct FileDirNode));
	strcpy(root->file_name, "/");
	root->file_num = 0;
	root->next = 0;
	root->is_dir=1;
	int root_hash = BKDRHash("/", PARENT_HASH_LENGTH);
	printf("par_dirs[root_hash]=%d\n", par_dirs+7);
	par_dirs[0] = root;
	par_dirs[1] = root;
	par_dirs[2] = root;
	par_dirs[3] = root;
	par_dirs[4] = root;
	par_dirs[5] = root;
	par_dirs[6] = root;
	par_dirs[7] = root;
	//par_dirs[root_hash] = root;
	root = 0;
}

/**
 * 创建文件
 * @dir  目录名称
 * @file 文件名称
 * @return 返回创建文件结果码
 */
int create_file(char * dir, char *file)
{
	int status = 0;
	int dir_hash_code = BKDRHash(dir, PARENT_HASH_LENGTH);
	if (!strlen(par_dirs[dir_hash_code]->file_name))
	{
		return dir_not_exists;
	}
	else{
		struct FileDirNode *dir_node = &par_dirs[dir_hash_code];
		do{
			if (strcmp(dir_node ->file_name, dir) == 0)
			{
				int file_hash_code = BKDRHash(file, PARENT_HASH_LENGTH);
				struct FileDirNode *file_node = (struct FileDirNode *)malloc(sizeof(struct FileDirNode));
				strcpy(file_node ->file_name, file);

			}
		} while (1);
	}
	return 0;
}

/**
 * @parent_dir 父目录名称
 * @dir		    目录名称
 * 1. 父目录不存在
 * 2. 目录名称不合法(名称格式 长度)
 * 3.
 */
int create_dir(char *path){
	char *parent_dir = (char *)malloc(sizeof(char) * file_name_max_length);
	char *dir = (char *)malloc(sizeof(char) * file_name_max_length);
	parse_path(parent_dir, dir, path);
	int dir_hash_code = BKDRHash(parent_dir, PARENT_HASH_LENGTH);
	int file_hash_code = BKDRHash(dir, PARENT_HASH_LENGTH);
	int path_hash_code = BKDRHash(path, PARENT_HASH_LENGTH);
	struct FileDirNode *tmp_node = par_dirs[dir_hash_code];
	struct FileDirNode *parent_node, *child_node;
	while(1){
		if(!tmp_node)
			return dir_not_exists;
		if(strcmp(tmp_node->file_name, parent_dir) == 0)
			break;
		tmp_node = tmp_node->next;
	}
	parent_node = tmp_node;
	child_node = par_dirs[dir_hash_code];
	tmp_node = tmp_node->child + file_hash_code;
	while(tmp_node){
		if(strcmp(tmp_node->file_name, dir) == 0)
			return dir_exists;
		tmp_node = tmp_node->next;
	}
	tmp_node = (FileDirNode *)malloc(sizeof(struct FileDirNode));
	tmp_node->next = child_node->next;
	child_node = tmp_node;
	strcpy(tmp_node->file_name, dir);
	tmp_node->is_dir = 1;
	tmp_node->child = 0;
	tmp_node->file_num = 0;
	par_dirs[path_hash_code] = child_node;
	tmp_node = 0;
	child_node = 0;
	parent_node = 0;
	return create_success;
}

void del_file(char *file_name){

}

extern const int illegal_file_name;
//int main(){
//	char input[16] = "/a/b/c1";
//	char *p = strtok(input, ",,");
//	init();
//	while (p)
//	{
//		puts(p);
//		p = strtok(NULL, "/");
//	}
//	//printf("illegal_file_name = %d\n", a);
//	char *c = (char *)malloc(sizeof(char) * 10), *d = (char *)malloc(sizeof(char) * 10);
//	parse_path(c, d, "123");
//	puts(c);
//	puts(d);
//	//printf("%d", strlen(par_dirs[0]->fileName));
//	//int b;
//	//scanf("%d", &b);
//	return 0;
//}

void hehe(char *c){
	strcpy(c, "123");
}

//int main(){
////	char a[100]="/a.hehe";
////
//////	char *c = "123";
//////	puts(c);
////	char *m = (char *)malloc(sizeof(char)*256), *n=(char *)malloc(sizeof(char)*256);
////	parse_path(m, n, a);
////	puts(m);
////	puts(n);
////	return 0;
////}

int main(){
	init();
	printf("创建文件返回=%d\n", create_dir("/"));
	print_dir();
	return 0;
}
