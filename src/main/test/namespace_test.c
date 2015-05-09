/*
 * namespace_test.c
 *
 *  Created on: 2015年5月8日
 *      Author: ron
 */

#include "../namespace/namespace.h"

///**
// * print the file system directory
// */
//static void print_dir() {
//	int i = 0, j;
//	file_dir_node *f;
//	file_dir_node *c;
//	file_dir_node *cc;
//	for (; i < PARENT_HASH_LENGTH; i++) {
//		f = par_dirs[i];
//		while (f) {
//			printf("i = %d dirs = %s |", i, par_dirs[i]->file_name);
//			j = 0;
//			for (; j < CHILD_HASH_LENGTH; j++) {
//				c = *(f->child + j);
//				//printf("c====%d ", c);
//				while (c) {
//					printf("%s ", c->file_name/*, *c->next_file*/);
//					c = c->next_file;
//				}
//			}
//			f = f->next_dir;
//		}
//		if (par_dirs[i])
//			puts("");
//	}
//	f = 0;
//	c = 0;
//}


void test_namespace_create(){

}

int main(){
	return 0;
}

