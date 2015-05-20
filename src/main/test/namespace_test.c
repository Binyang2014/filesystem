/*
 * namespace_test.c
 *	unit test for name space
 *  Created on: 2015年5月8日
 *      Author: ron
 */

#include "../namespace/namespace.h"
#include "../../tool/errinfo.h"

static void print_namesapce(namespace *namespace){
	int i = 0;
	while(i != namespace->parent_hash_length){
		file_dir_node *node = *(namespace->parent_dirs + i);
		while(node != NULL){
			printf("directory name = \"%s\" sub file num = %d ->", node->file_name, node->file_num);
			int j = 0;
			while(j != namespace->child_hash_length){
				file_dir_node *child_node = *(node->child + j);
//				if(strcmp(node->file_name,"/") == 0)
//					printf("root child %d %s\n", j, child_node);
				while(child_node != NULL){
					printf(" %s", child_node->file_name);
					child_node = child_node->next_file;
				}
				j++;
			}
			node = node->next_dir;
			puts("");
		}
		i++;
	}
}


static void test_create_dir(namespace *namespace, char *name){
	//err_ret("-----before create dir-----");
	int status = namespace_create_dir(namespace, name);
	printf("create dir status = %d\n", status);
	//err_ret("-----after create dir-----");
}

static void test_create_file(namespace *namespace, char *name){
	//err_ret("-----before create file-----");
	int status = namespace_create_file(namespace, name);
	printf("create file status = %d\n", status);
	//err_ret("-----after create file-----");
}


static void test_namespace_create(){
	char first_directory[100] = "/heallo/";
	char first_file[1000] = "/readin";
	namespace *namespace = create_namespace(1024, 8);
	//test_create_dir(namespace, first_directory);
	//test_create_file(namespace, first_file);
	test_create_file(namespace, first_file);
	print_namesapce(namespace);
}


int main(){
	test_namespace_create();
	return 0;
}

