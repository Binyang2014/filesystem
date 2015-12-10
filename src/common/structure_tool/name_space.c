/*
 * name_space.c
 *
 *  Created on: 2015年7月9日
 *      Author: ron
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include "name_space.h"
#include "zmalloc.h"
#include "message.h"

/*-------------------Private Declaration----------------------*/
static file_node_t *find(name_space_t *space, sds name);
static int add_temporary_file(name_space_t *space, sds file_name);
static int rename_file(name_space_t *space, sds old_name, sds new_name);
static int delete_file(name_space_t *space, sds file_name);
static int file_finish_consistent(struct name_space *space, sds file_name);
static int file_exists(struct name_space *space, sds file_name);
static void print_name_space(struct name_space *space);

/*-------------------Local Implementation--------------------*/
static void *list_dup(void *ptr)
{
	pair_t *pair = zmalloc(sizeof(*pair));
	pair_t *tmp = ptr;
	pair->key = sds_dup(tmp->key);
	file_node_t *node = zmalloc(sizeof(*node));
	memcpy(node, tmp->value, sizeof(*node));
	pair->value = node;
	return pair;
}

static void list_free(void *ptr)
{
	pair_t *pair = ptr;
	sds_free(pair->key);
	zfree(pair->value);
}

static file_node_t *add_file(name_space_t *space, sds file_name, enum file_type_enum type)
{
	file_node_t *node = find(space, file_name);
	if(node != NULL) {
		return NULL; //TODO
	}

	node = zmalloc(sizeof(file_node_t));
	node->file_name = sds_new(file_name);
	node->file_size = 0;
	node->file_type = type;
	node->consistent_size = 0;
	space->file_num++;
	return node;
}

/*-------------------Private Implementation--------------------*/
static file_node_t *find(name_space_t *space, sds name)
{
	return space->name_space->op->get(space->name_space, name);
}

static int add_temporary_file(name_space_t *space, sds file_name)
{
	file_node_t *node = add_file(space, file_name, TEMPORARY_FILE);
	if(node == NULL)
	{
		return FILE_EXISTS;
	}

	space->name_space->op->put(space->name_space, file_name, node);
	return FILE_OPERATE_SUCCESS;
}

static int add_persistent_file(name_space_t *space, sds file_name)
{
	int file_exists = access(file_name, F_OK);
	if(file_exists) {
		return FILE_EXISTS;
	}

	file_node_t *node = add_file(space, file_name, PERSISTENT_FILE);
	FILE *fp = fopen(file_name, "ab+");
	node->fp = fp;
	space->name_space->op->put(space->name_space, file_name, node);
	//TODO check if file has closed
	return FILE_OPERATE_SUCCESS;
}

static int append_file(name_space_t *space, sds file_name, uint64_t append_size)
{
	file_node_t *node = find(space, file_name);
	if(node == NULL) {
		return FILE_NOT_EXISTS;
	}

	node->file_size +=append_size;
	return FILE_OPERATE_SUCCESS;
}

static int rename_file(name_space_t *space, sds old_name, sds new_name)
{
	return space->name_space->op->modify_key(space->name_space, old_name, new_name);
}

static int delete_file(name_space_t *space, sds file_name)
{
	return space->name_space->op->del(space->name_space, file_name);
}

static int file_finish_consistent(struct name_space *space, sds file_name)
{
	file_node_t *node = find(space, file_name);

	if(node == NULL) {
		return -1;
	}

	return node->consistent_size == node->file_size;
}

static int file_exists(struct name_space *space, sds file_name)
{
	return find(space, file_name) != NULL;
}

static void print_name_space(struct name_space *space)
{
	printf("\n==========START PRINT NAME SPACE==========");
	printf("space->file_num = %d", space->file_num);
	map_iterator_t *iter = create_map_iterator(space->name_space);
	while(iter->op->has_next(iter)){
		file_node_t *node = iter->op->next(iter);
		printf("file name is %s and file size is %d\n", node->file_name, node->file_size);
	}

	destroy_map_iterator(iter);
	printf("\n==========END PRINT NAME SPACE==========");
}

static list_t *get_file_location(name_space_t *space, sds file_name)
{
	file_node_t *node = find(space, file_name);

	assert(node != NULL);

	return node->position;
}

static void set_file_location(name_space_t *space, sds file_name, list_t *list)
{
	file_node_t *node = find(space, file_name);

	assert(node != NULL);

	if(node->position == NULL){
		node->position = list;
	}else{
		list_t *list_head = node->position;
		list_head->list_ops->list_merge_list(list_head, list);
	}
}

struct file_node* get_file_node(name_space_t *space, sds file_name)
{
	return find(space, file_name);
}


/*--------------------API Implementation-------------------*/
name_space_t *create_name_space(size_t size)
{
	name_space_t *this = zmalloc(sizeof(*this));

	this->name_space = create_map(size, NULL, NULL, list_dup, list_free);
	this->file_num = 0;
	this->op = zmalloc(sizeof(name_space_op_t));
	this->op->add_persistent_file = add_persistent_file;
	this->op->add_temporary_file =  add_temporary_file;
	this->op->delete_file = delete_file;
	this->op->file_finish_consistent = file_finish_consistent;
	this->op->rename_file = rename_file;
	this->op->get_file_location = get_file_location;
	this->op->set_file_location = set_file_location;
	this->op->get_file_node = get_file_node;
	this->op->file_exists = file_exists;
	this->op->append_file = append_file;
	this->op->print_name_space = print_name_space;
	return this;
}

void destroy_name_space(name_space_t *this)
{
	zfree(this->op);
	destroy_map(this->name_space);
	zfree(this);
}
