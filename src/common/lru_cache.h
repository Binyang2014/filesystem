/*
 * cache.h
 *
 *  Created on: 2015年6月25日
 *      Author: ron
 */

#ifndef CACHE_H_
#define CACHE_H_
#include <time.h>
#include "map.h"
#include "basic_list.h"

struct lru_cache{
	list_t *list;
	map_t *map;
	struct lru_cache_op *op;
	size_t max_size;
	void *(*value_dup)(const void *value);
	void *(*value_free)(sds *value);
};

struct lru_cache_op{
	void *(*get)(struct lru_cache *cache, sds *key);
	void (*put)(struct lru_cache *cache, sds *key, void *value);
};

#define lru_cache_value_dup(l, p) \
		(l)->value_dup = p;	\
		(l)->list->dup = p;
#define lru_cache_value_free(l, p) \
		(l)->value_free = p; \
		(l)->list_free = p;

typedef struct lru_cache lru_cache_t;
typedef struct lru_cache_op lru_cache_op_t;

lru_cache_t *create_lru_cache(size_t size);
void destroy_lru_cache(lru_cache_t *this);
#endif /* CACHE_H_ */
