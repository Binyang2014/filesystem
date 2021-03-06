/*
 *created on: 2015.10.20
 *author: binyang
 */
#include <stdio.h>
#include "client_struct.h"
#include "zmalloc.h"

static void free_data_node(void *args)
{
	data_node_t *data_node;

	data_node = args;
	zfree(data_node->chunks_id);
	zfree(data_node);
}

int match_file(void *ptr, void *key)
{
	opened_file_t *opened_file;
	int *fd;

	opened_file = ptr;
	fd = key;
	if(opened_file->fd == *fd)
		return 1;
	else
		return 0;
}

opened_file_t *create_openedfile(const char *file_path, int position, open_mode_t
		open_mode)
{
	opened_file_t *open_file = NULL;

	open_file = zmalloc(sizeof(opened_file_t));
	if(open_file == NULL)
		return NULL;
	open_file->open_mode = open_mode;
	open_file->position = position;
	open_file->f_info.data_nodes = list_create();
	list_set_free_method(open_file->f_info.data_nodes, free_data_node);
	open_file->f_info.file_path = sds_new(file_path);
	open_file->f_info.file_offset = 0;
	open_file->version = 0;

	return open_file;
}

void free_file(void *args)
{
	opened_file_t *opened_file;

	opened_file = args;
	list_release(opened_file->f_info.data_nodes);
	sds_free(opened_file->f_info.file_path);

	zfree(opened_file);
}
