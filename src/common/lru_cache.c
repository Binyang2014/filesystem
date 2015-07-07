/*
 ============================================================================
 Name        : mpi_cache.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <assert.h>
#include "lru_cache.h"
#include "zmalloc.h"

#define LOAD_MODE 2

/*
typedef struct cache_node {
	sds key;
	void *value;
}cache_node_t;

static cache_node_t *create_cache_node(lru_cache_t *this, const sds key, const void *value) {
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

static void *map_v_dup(void *v) {
	void **ptr = zmalloc(sizeof(void *));
	*ptr = *(void **)v;
	return ptr;
}

/**
 * map value dup
 */
static void *lru_cache_pair_dup(void *pair) {
	pair_t *p = zmalloc(sizeof(*pair));
	p->key = sds_dup(((pair_t *)pair)->key);
	p->value = map_v_dup(((pair_t *)pair)->value);
	return p;
}

static void lru_cache_pair_free(void *pair) {
	pair_t *p = pair;
	sds_free(p->key);
	zfree(p->value);
}

static void put(lru_cache_t *this, sds key, void *value) {
	void *v = this->map->op->get(this->map, key);
	list_node_t **node = v;
	//printf("put find %d key = %s\n", v, key);

	pair_t *p = zmalloc(sizeof(pair_t));
	p->key = sds_dup(key);
	p->value = this->value_dup(value);

	if(v == NULL) {
		if(this->max_size == this->list->len) {
			//if full, delete list head,
			this->map->op->del(this->map, ((pair_t *)(this->list->head->value))->key);
			this->list->list_ops->list_del_node(this->list, this->list->head);
		}
		this->list->list_ops->list_add_node_tail(this->list, p);
	}else {
		//delete old key-value in list
		this->list->free((*node)->value);
		(*node)->value = p;
		this->list->list_ops->list_extract_node_to_tail(this->list, *node);
	}

	//add the new node to the list and add new list node address to the map(key, new address)
	this->map->op->put(this->map, key, &(this->list->tail));

	assert(this->list->len == this->map->current_size);
}

/**
 * get a copy of the value
 */
static void *get(lru_cache_t *this, sds key) {
	void *v = this->map->op->get(this->map, key);
	//printf("get find %d\n", v);

	if(v == NULL) {
		return NULL;
	}else {
		list_node_t **node = v;
		this->list->list_ops->list_extract_node_to_tail(this->list, *node);
		//printf("get %d\n", node);
		//printf("get %d %d %d %d\n", node, this->list->head, this->list->tail, *(int *)((pair_t *)this->list->tail->value)->value);
		return this->value_dup(((pair_t *)(*node)->value)->value);
	}
}

static void del(lru_cache_t *this, sds key) {
	void *v = this->map->op->get(this->map, key);

	if(v == NULL) {
		return;
	}else {
		this->map->op->del(this->map, key);
		this->list->list_ops->list_del_node(this->list, *(list_node_t **)v);
	}
}

static size_t get_size(lru_cache_t *cache) {
	return cache->list->len;
}

/**
 * map put list node address
 */
lru_cache_t *create_lru_cache(size_t size, void *(*value_dup)(const void *value), void *(*value_free)(sds value),
		void *(*pair_dup)(const void *value), void (*pair_free)(void *value)) {
	assert(size > 0);

	lru_cache_t *this = zmalloc(sizeof(lru_cache_t));
	this->op = zmalloc(sizeof(lru_cache_op_t));

	this->map = create_map(size * LOAD_MODE);
	set_map_value_dup(this->map, map_v_dup);
	set_map_value_free(this->map, zfree);
	set_map_list_pair_dup(this->map, lru_cache_pair_dup);
	set_map_list_pair_free(this->map, lru_cache_pair_free);

	this->max_size = size;
	this->list = list_create();

	this->op->get = get;
	this->op->put = put;
	this->op->get_size = get_size;
	this->op->del = del;

	this->list->dup = pair_dup;
	this->list->free = pair_free;
	this->value_dup = value_dup;
	this->value_free = value_free;

	return this;
}

void destroy_lru_cache(lru_cache_t *this) {
	zfree(this->op);
	destroy_map(this->map);
	zfree(this);
}

#if 0

void v_free(void *v) {
	zfree(v);
}

void *v_dup(void *v){
	int *t = zmalloc(sizeof(int));
	*t = *((int *)v);
	return (void *)t;
}

static void *list_pair_dup(const void *value){
	pair_t *p = zmalloc(sizeof(pair_t));
	p->key = sds_dup(((pair_t *)value)->key);
	//this is your value dup function, must be specify
	p->value = zmalloc(sizeof(int));
	*(int *)p->value = *(int *)(((pair_t *)value)->value);
	return p;
}

static list_pair_free(void *value){
	pair_t *p = value;
	sds_free(p->key);
	zfree(p->value);
}

int main(){
	lru_cache_t *this = create_lru_cache(5, v_dup, v_free, list_pair_dup, list_pair_free);

	sds s = sds_new("1234");
	int num = 1234;
	this->op->put(this, s, &num);
	int *t = this->op->get(this, s);
	printf("%d\n", *t);

	sds s2 = sds_new("2345");
	num = 2345;
	this->op->put(this, s2, &num);
	t = this->op->get(this, s2);
	printf("%d\n", *t);

	sds s3 = sds_new("3456");
	num = 3456;
	this->op->put(this, s3, &num);
	t = this->op->get(this, s3);
	printf("%d\n", *t);

	sds s4 = sds_new("4567");
	num = 4567;
	this->op->put(this, s4, &num);
	t = this->op->get(this, s4);
	printf("%d\n", *t);

	sds s5 = sds_new("5678");
	num = 5678;
	this->op->put(this, s5, &num);
	t = this->op->get(this, s5);
	printf("%d\n", *t);

	num = 1111;
	this->op->put(this, s5, &num);
	t = this->op->get(this, s5);
	printf("%d size = %d\n", *t, this->op->get_size(this));
	return 0;

}
#endif
