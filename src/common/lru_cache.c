/*
 ============================================================================
 Name        : mpi_cache.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "lru_cache.h"
#include "zmalloc.h"

#define LOAD_MODE 2

/*
typedef struct cache_node {
	sds *key;
	void *value;
}cache_node_t;

static cache_node_t *create_cache_node(lru_cache_t *this, const sds *key, const void *value) {
	cache_node_t *node = zmalloc(sizeof(cache_node_t));

	node->key = this->key_dup(key);
	node->value = this->value_dup(value);
	return node;
}
static cache_node_t *destroy_cache_node(lru_cache_t *this, cache_node_t *node) {
	this->key_free(node->key);
	this->value_free(node->value);
	zfree(node);
}
*/

static void *get(lru_cache_t *this, sds *key) {
	void *v = this->map->op->get(this->map, key);

	if(v == NULL) {
		return NULL;
	}else {
		list_node_t *node = v;
		this->list->list_ops->list_add_node_tail(this->list, node->value);
		this->list->list_ops->list_del_node(this->list, node);
		return node->value;
	}
}

static void put(lru_cache_t *this, sds *key, void *value) {
	void *v = this->map->op->get(this->map, key);

	if(v == NULL) {
		if(this->max_size == this->list->len) {
			//if full, delete list head,
			this->list->list_ops->list_del_node(this->list, this->list->head);
			this->map->op->del(this->map, key);
		}
		//add the new node to the list and add new list node address to the map(key, new address)
		this->list->list_ops->list_add_node_tail(this->list, value);
		this->map->op->put(key, &(this->list->tail));
	}else {
		list_node_t *node = v;
		this->list->list_ops->list_add_node_tail(this->list, node->value);
		this->list->list_ops->list_del_node(this->list, node);
	}

	assert(this->list->len == this->map->current_size);
}

lru_cache_t *create_lru_cache(size_t size) {
	lru_cache_t *this = zmalloc(sizeof(lru_cache_t));
	this->op = zmalloc(sizeof(lru_cache_op_t));
	this->map = create_map(size * LOAD_MODE);
	this->max_size = size;

	this->list = list_create();

	return this;
}

void destroy_lru_cache(lru_cache_t *this) {
	zfree(this->op);
	destroy_map(this->map);
	zfree(this);
}

#ifdef LRU_CACHE_TEST

int main(){
	return 0;
}
#endif
