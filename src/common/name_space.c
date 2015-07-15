/*
 * name_space.c
 *
 *  Created on: 2015年7月9日
 *      Author: ron
 */


#include <stdio.h>
#include <stdlib.h>
#include "name_space.h"

/*-------------------Private Implementation--------------------*/
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

static file_node_t *find(name_space_t *space, sds name) {
	return space->name_space->op->get(space->name_space, name);
}

static int add_file(name_space_t *space, sds file_name, enum file_type_enum type) {
	file_node_t *node = find(space, file_name);
	if(node != NULL) {
		return -1;//TODO
	}

	node = zmalloc(sizeof(file_node_t));
	sds_cpy(node->file_name, file_name);
	node->file_size = 0;
	node->file_type = type;
	node->consistent_size = 0;
	space->file_num++;
	return 1;
}

static int add_temporary_file(name_space_t *space, sds file_name) {
	return add_file(space, file_name, TEMPORARY_FILE);
}

static int add_persistent_file(name_space_t *space, sds file_name) {
	return add_file(space, file_name, PERSISTENT_FILE);
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