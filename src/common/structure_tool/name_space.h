/*
 * namespace.h
 *
 *  Created on: 2015年7月9日
 *      Author: ron
 */

#ifndef SRC_COMMON_NAME_SPACE_H_
#define SRC_COMMON_NAME_SPACE_H_

#include <pthread.h>
#include "sds.h"
#include "map.h"

enum file_type_enum {
	PERSISTENT_FILE,
	TEMPORARY_FILE
};

struct file_node {
	uint8_t file_type; //{0:persistent, 1:temporary}
	sds file_name;
	uint64_t consistent_size;
	uint64_t file_size;
	list_t *position;
	FILE *fp;
};

struct name_space_op {
	int (*add_temporary_file)(struct name_space space, sds file_name);
	int (*add_persistent_file)(struct name_space space, sds file_name);
	int (*rename_file)(struct name_space space, sds old_name, sds new_name);
	int (*delete_file)(struct name_space space, sds file_name);
	int (*file_finish_consistent)(struct name_space *space, sds file_name);
	int (*file_exists)(struct name_space *space, sds file_name);
	int (*append_file)(name_space_t *space, sds file_name, uint64_t append_size);
	list_t *(*get_file_location)(name_space_t *space, sds file_name);
	int (*set_file_location)(name_space_t *space, sds file_name, list_t *list);
};

struct name_space {
	map_t *name_space;
	size_t file_num;
	struct name_space_op *op;
};

typedef struct name_space name_space_t;
typedef struct name_space_op name_space_op_t;
typedef struct file_node file_node_t;
typedef struct name_space_file_location n_s_f_l_t;

name_space_t *create_name_space(size_t size);
void destroy_name_space(name_space_t *this);

#endif /* SRC_COMMON_NAME_SPACE_H_ */
