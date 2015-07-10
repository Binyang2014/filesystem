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

struct name_space_file_location {

};

struct file_node {
	uint8_t file_type; //{0:persistent, 1:temporary}
	sds file_name;
	uint64_t file_size;
	struct name_space_file_location *location;

};

struct name_space_op {
	int (*add_temporary_file)(struct name_space space, sds file_name);
	int (*add_persistent_file)(struct name_space space, sds file_name);
	int (*rename_file)(struct name_space space, sds old_name, sds new_name);
	struct file_ndoe *(*get_file_node)(struct name_space space, sds file_name);
	int (*delete_file)(struct name_space space, sds file_name);
};

struct name_space {
	map_t *name_space;
	size_t file_num;
	struct name_space_op *op;
};

typedef struct name_space name_space_t;
typedef struct file_node file_node_t;
typedef struct name_space_file_location n_s_f_l_t;

name_space_t *create_name_space(size_t size);
void destroy_name_space(name_space_t *this);

#endif /* SRC_COMMON_NAME_SPACE_H_ */
