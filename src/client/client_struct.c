/*
 *created on: 2015.10.20
 *author: binyang
 */
#include "client_struct.h"
#include "zmalloc.h"

opened_file_t *create_file(const char *file_path, int positon, open_mode_t
		open_mode)
{
	opened_file_t *open_file = NULL;

	open_file = zmalloc(sizeof(opened_file_t));
	if(open_file == NULL)
		return NULL;
	open_file->open_mode = open_mode;
	open_file->position = position;
	open_file->f_info->data_nodes = list_create();
	list_set_free_method(open_file->f_info->data_nodes, zfree);
	open_file->f_info->file_path = sds_new(file_path);
	open_file->f_info->file_offset = 0;
	open_file->version = 0;

	return open_file;
}

void free_file(opened_file_t *opened_file)
{
	list_release(open_file->f_info->data_nodes);
	sds_free(open_file->f_info->file_path);

	zfree(opened_file);
}
