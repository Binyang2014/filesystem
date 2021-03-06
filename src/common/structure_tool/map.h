/*
 * map.h
 *
 *  Created on: 2015.07.02
 *      Author: ron
 *
 *    A fix-size map
 */

#ifndef SRC_MAP_H_
#define SRC_MAP_H_
#include <stdint.h>
#include <stddef.h>
#include "sds.h"
#include "basic_list.h"
#include "global.h"

struct pair {
	sds key;
	void *value;
};

typedef struct map {
	list_t **list;
	struct map_op *op;
	size_t current_size;
	size_t size;
	sds (*key_dup)(const sds key);
	void (*key_free)(sds key);
	void *(*value_dup)(const void *value);
	void (*value_free)(void *value);
	list_t *key_list;
}map_t;

struct map_iterator {
	struct map *map;
	int dir_no;
	list_t *list;
	list_iter_t *iter;
	struct map_iterator_op *op;
};

struct map_iterator_op {
	int (*has_next)(struct map_iterator *iterator);
	void *(*next)(struct map_iterator *iterator);
};


struct map_op {
	int (*put)(map_t *map, const sds key, void *value);
	void *(*get)(map_t *map, sds key);
	int (*contains)(map_t *map, sds key);
	int (*modify_key)(map_t *map, sds old_key, sds new_key);
	size_t (*get_size)(map_t *map);
	int (*del)(map_t *map, sds key);
	sds *(*get_all_keys)(map_t *map, int *count);
};

typedef struct pair pair_t;
typedef struct map_op map_op_t;
typedef struct map_iterator map_iterator_t;
typedef struct map_iterator_op map_iterator_op_t;

map_t *create_map(size_t size, void *(*value_dup)(const void *value), void (*value_free)(void *value),
		void *(*list_dup)(const void *ptr), void (*list_free)(void *ptr));
void destroy_map(map_t *map);
map_iterator_t *create_map_iterator(map_t *map);
void destroy_map_iterator(map_iterator_t *iter);

#endif /* SRC_MAP_H_ */
