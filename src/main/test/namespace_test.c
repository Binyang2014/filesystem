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
			printf("%s->", node->file_name);
			int j = 0;
			while(j != namespace->child_hash_length){
				file_dir_node *child_node = *(node->child + j);
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
	err_ret("-----before create dir-----");
	namespace_create_dir(namespace, name);
	print_namesapce(namespace);
	err_ret("-----after create dir-----");
}

static void test_create_file(namespace *namespace, char *name){
	err_ret("-----before create file-----");
	namespace_create_file(namespace, name);
	print_namesapce(namespace);
	err_ret("-----after create file-----");
}

static void test_namespace_create(){
	char *first_directory = "/hello";
	namespace *namespace = create_namespace(1024, 32);
	//printf("%d\n", namespace);
	//err_ret("")
	test_create_dir(namespace, first_directory);
	//print_namesapce(namespace);
}

int main(){
	test_namespace_create();
	return 0;
}

