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

struct pair{
	sds key;
	void *value;
};

typedef struct map{
	list_t **list;
	struct map_op *op;
	size_t current_size;
	size_t size;
	sds (*key_dup)(const sds key);
	void *(*value_dup)(const void *value);
	void (*key_free)(sds key);
	void (*value_free)(void *value);
    void *(*list_dup)(const void *ptr);
    void (*list_free)(void *ptr);
}map_t;

struct map_op{
	int (*put)(map_t *map, const sds key, const void *value);
	void *(*get)(map_t *map, sds key);
	int (*contains)(map_t *map, sds key);
	size_t (*get_size)(map_t *map);
	void (*del)(map_t *map, sds key);
};

typedef struct pair pair_t;
typedef struct map_op map_op_t;

#define set_map_value_dup(m, p) ((m)->value_dup) = p
#define set_map_value_free(m, p) ((m)->value_free) = p
#define set_map_list_pair_dup(m, p)	\
		int _i = 0;	\
		for(_i = 0; _i < ((m)->size); _i++) { \
			(*((m)->list + _i))->dup = p; \
		} \

#define set_map_list_pair_free(m, p) \
		int _j = 0;	\
		for(_j = 0; _j < ((m)->size); _j++) { \
			(*((m)->list + _j))->free = p; \
		} \

map_t *create_map(size_t size);
void destroy_map(map_t *map);
#endif /* SRC_MAP_H_ */
