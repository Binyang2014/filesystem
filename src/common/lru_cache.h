/*
 * cache.h
 *
 *  Created on: 2015年6月25日
 *      Author: ron
 *
 * Note: when use this structure, one must provide value dup and free function as well as (key, value) dup and free function
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
	void (*value_free)(void *value);
};

struct lru_cache_op{
	void *(*get)(struct lru_cache *cache, sds key);
	void (*put)(struct lru_cache *cache, sds key, void *value);
	size_t (*get_size)(struct lru_cache *cache);
	void (*del)(struct lru_cache *cache, sds key);
};

typedef struct lru_cache lru_cache_t;
typedef struct lru_cache_op lru_cache_op_t;

lru_cache_t *create_lru_cache(size_t size, void *(*value_dup)(const void *value), void (*value_free)(void *value),
		void *(*pair_dup)(void *value), void (*pair_free)(void *value));
void destroy_lru_cache(lru_cache_t *this);
#endif /* CACHE_H_ */
