/*
 * name_space.c
 *
 *  Created on: 2015年7月9日
 *      Author: ron
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "name_space.h"

#define FILE_EXISTS 400
#define FILE_NOT_EXISTS 404
#define FILE_OPERATE_SUCCESS 0

/*-------------------Private Declaration----------------------*/
static file_node_t *find(name_space_t *space, sds name);
static int add_temporary_file(name_space_t *space, sds file_name);
static int rename_file(name_space_t *space, sds old_name, sds new_name);
static int delete_file(name_space_t *space, sds file_name);
static int file_finish_consistent(struct name_space *space, sds file_name);
static int file_exists(struct name_space *space, sds file_name);

/*-------------------Local Implementation--------------------*/
static void *list_dup(void *ptr) {
	pair_t *pair = zmalloc(sizeof(*pair));
	pair_t *tmp = ptr;
	pair->key = sds_dup(tmp->key);
	file_node_t *node = zmalloc(sizeof(*node));
	memcpy(node, tmp->value, sizeof(*node));
	pair->value = node;
	return pair;
}

static void list_free(void *ptr) {
	pair_t *pair = ptr;
	sds_free(pair->key);
	zfree(pair->value);
}

static file_node_t *add_file(name_space_t *space, sds file_name, enum file_type_enum type) {
	file_node_t *node = find(space, file_name);
	if(node != NULL) {
		return NULL; //TODO
	}

	node = zmalloc(sizeof(file_node_t));
	sds_cpy(node->file_name, file_name);
	node->file_size = 0;
	node->file_type = type;
	node->consistent_size = 0;
	space->file_num++;
	return node;
}

/*-------------------Private Implementation--------------------*/
static file_node_t *find(name_space_t *space, sds name) {
	return space->name_space->op->get(space->name_space, name);
}

static int add_temporary_file(name_space_t *space, sds file_name) {
	file_node_t *node = add_file(space, file_name, TEMPORARY_FILE);
	if(node == NULL) {
		return FILE_EXISTS;
	}

	space->name_space->op->put(file_name, node);
	return FILE_OPERATE_SUCCESS;
}

static int add_persistent_file(name_space_t *space, sds file_name) {
	int file_exists = access(file_name, F_OK);
	if(file_exists) {
		return FILE_EXISTS;
	}

	file_node_t *node = add_file(space, file_name, PERSISTENT_FILE);
	FILE *fp = fopen(file_name, "ab+");
	node->fp = fp;
	space->name_space->op->put(file_name, node);
	return FILE_OPERATE_SUCCESS;
}

static int append_file(name_space_t *space, sds file_name, uint64_t append_size) {
	file_node_t *node = find(space, file_name);
	if(node == NULL) {
		return FILE_NOT_EXISTS;
	}

	node->file_size +=append_size;
	return FILE_OPERATE_SUCCESS;
}

static int rename_file(name_space_t *space, sds old_name, sds new_name) {
	return space->name_space->op->modify_key(space->name_space, old_name, new_name);
}

static int delete_file(name_space_t *space, sds file_name) {
	return space->name_space->op->del(space->name_space, file_name);
}

static int file_finish_consistent(struct name_space *space, sds file_name) {
	file_node_t *node = find(space, file_name);

	if(node == NULL) {
		return -1;
	}

	return node->consistent_size == node->file_size;
}

static int file_exists(struct name_space *space, sds file_name) {
	return find(space, file_name) != NULL;
}

list_t *get_file_location(name_space_t *space, sds file_name){
	file_node_t *node = find(space, file_name);

	assert(node != NULL);

	return node->position;
}

void set_file_location(name_space_t *space, sds file_name, list_t *list){
	file_node_t *node = find(space, file_name);

	assert(node != NULL);

	if(node->position == NULL){
		node->position = list;
	}else{
		//node->position->list_ops->list_
	}
}


/*--------------------API Implementation-------------------*/
name_space_t *create_name_space(size_t size) {
	name_space_t *this = zmalloc(sizeof(*this));

	this->name_space = create_map(size, NULL, NULL, list_dup, list_free);
	this->file_num = 0;
	this->op = zmalloc(sizeof(name_space_op_t));
	this->op->add_persistent_file = add_persistent_file;
	this->op->add_temporary_file =  add_temporary_file;
	this->op->delete_file = delete_file;
	this->op->file_finish_consistent = file_finish_consistent;
	this->op->rename_file = rename_file;

	return this;
}

void destroy_name_space(name_space_t *this) {
	zfree(this->op);
	destroy_map(this->name_space);
	zfree(this);
}


/*------------------T	E	S	T-----------------*/

#if defined(GLOBAL_TEST) || defined(NAME_SPACE_TEST)
int main() {

}
#endif
