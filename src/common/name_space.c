/*
 * name_space.c
 *
 *  Created on: 2015年7月9日
 *      Author: ron
 */


#include <stdio.h>
#include <stdlib.h>
#include "name_space.h"

static file_node_t *find(name_space_t *space, sds name) {
	return space->name_space->op->get(space->name_space, name);
}

static int add_file(name_space_t *space, sds file_name, enum file_type_enum type) {
	file_node_t *node = find(space, file_name);
	if(node != NULL) {
		return 00;//TODO
	}

	node = zmalloc(sizeof(file_node_t));
	sds_cpy(node->file_name, file_name);
	node->file_size = 0;
	node->file_type = type;

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

static file_node_t *get_file_node(name_space_t *space, sds file_name) {
	return space->name_space->op->get(space->name_space, file_name);;
}

int delete_file(name_space_t *space, sds file_name) {
	return space->name_space->op->del(space->name_space, file_name);
}

name_space_t *create_name_space(size_t size) {

}

void destroy_name_space(name_space_t *this) {

}
