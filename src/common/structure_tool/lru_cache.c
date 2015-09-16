/*
 ============================================================================
 Name        : mpi_cache.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style

 Map store (key, list node address)
 List store key-value  pair
 ============================================================================
 */

#include <assert.h>
#include "lru_cache.h"
#include "zmalloc.h"

#define LOAD_MODE 2

/*--------------------Private Declaration------------------*/

/*--------------------API Declaration----------------------*/

/*--------------------Private Implementation---------------*/

static void lru_cache_pair_free(void *pair) {
	pair_t *p = pair;
	sds_free(p->key);
	zfree(p->value);
	zfree(p);
}

/*
 * first check if map contaions key
 *
 * if contains key
 * 		extract list no to tail, free old value(Is there any consistent problem?), update list pair value,
 * else
 * 		if FULL
 * 			delete list head and delete head (key, value) in map
 * 		add key value to list tail and put (key, list node address) to map
 *
 * 	There is a problem, if cache is full, new node will replace one old node and old node will be released,
 * 	object who owns the old node value will make error, so we strongly recommend to transport a value_dup function
 * 	to the cache, on which we will try to return a value dup_copy to the object.
 */
static void put(lru_cache_t *this, sds key, void *value) {
	void *v = this->map->op->get(this->map, key);

	pair_t *p = zmalloc(sizeof(pair_t));
	p->key = sds_dup(key);
	p->value = this->value_dup ? this->value_dup(value) : value;

	if(v == NULL) {
		if(this->max_size == this->list->len) {
			//if full, delete list head,
			this->map->op->del(this->map, ((pair_t *)(this->list->head->value))->key);
			this->list->list_ops->list_del_node(this->list, this->list->head);
		}
		this->list->list_ops->list_add_node_tail(this->list, p);
	}else {
		//delete old key-value in list
		list_node_t **node = v;

		if(this->list->free) {
			this->list->free((*node)->value);
		}
		else {
			zfree((*node)->value); //free old value
		}

		(*node)->value = p;
		this->list->list_ops->list_extract_node_to_tail(this->list, *node);
	}

	//add the new node to the list and add new list node address to the map(key, new address)
	void **ptr = zmalloc(sizeof(void *));
	*ptr = this->list->tail;
	this->map->op->put(this->map, key, (void *)ptr);
	assert(this->list->len == this->map->current_size);
}

static void *get(lru_cache_t *this, sds key) {
	void *v = this->map->op->get(this->map, key);
	if(v == NULL) {
		return NULL;
	}else {
		list_node_t **node = v;
		this->list->list_ops->list_extract_node_to_tail(this->list, *node);
		void *result =  ((pair_t *)((*node)->value))->value;
		return this->value_dup ? this->value_dup(result) : result;
	}
}

//static void del(lru_cache_t *this, sds key) {
//	void *v = this->map->op->get(this->map, key);
//
//	if(v == NULL) {
//		return;
//	}else {
//		this->map->op->del(this->map, key);
//		this->list->list_ops->list_del_node(this->list, *(list_node_t **)v);
//	}
//}

static size_t get_size(lru_cache_t *cache) {
	return cache->list->len;
}

/*--------------------API Implementation-------------------*/

/**
 * map put list node address
 */
lru_cache_t *create_lru_cache(size_t size, void *(*value_dup)(const void *value), void (*value_free)(void *value),
		void *(*pair_dup)(void *value), void (*pair_free)(void *value)) {
	assert(size > 0);

	lru_cache_t *this = zmalloc(sizeof(lru_cache_t));
	this->op = zmalloc(sizeof(lru_cache_op_t));

	this->map = create_map(size * LOAD_MODE, NULL, NULL, NULL, lru_cache_pair_free);

	this->max_size = size;
	this->list = list_create();

	this->op->get = get;
	this->op->put = put;
	this->op->get_size = get_size;
	//this->op->del = del;

	this->list->dup = pair_dup;
	this->list->free = pair_free;
	this->value_dup = value_dup;
	this->value_free = value_free;

	return this;
}

void destroy_lru_cache(lru_cache_t *this) {
	zfree(this->op);
	destroy_map(this->map);
	//printf("len = %d\n", this->list->len);
	list_release(this->list);
	zfree(this);
}

/*------------------T	E	S	T----------------*/
//#define CACHE_TEST 1
#if defined(GLOBAL_TEST) || defined(CACHE_TEST)
#include <stdio.h>

static void list_pair_free(void *value){
	pair_t *p = value;
	sds_free(p->key);
	zfree(p->value);
	zfree(p);
}

int main(){
	size_t size = 5;
	lru_cache_t *this = create_lru_cache(size, NULL, NULL, NULL, list_pair_free);

	sds s = sds_new("1234");
	int *num = zmalloc(sizeof(int));
	*num = 1234;
	this->op->put(this, s, num);
	int *t = this->op->get(this, s);
	printf("s = %d size = %zu\n", *t, this->op->get_size(this));

	sds s2 = sds_new("2345");
	num = zmalloc(sizeof(int));
	*num = 2345;
	this->op->put(this, s2, num);
	t = this->op->get(this, s2);
	printf("s2 = %d size = %zu\n", *t, this->op->get_size(this));

	sds s3 = sds_new("3456");
	num = zmalloc(sizeof(int));
	*num = 3456;
	this->op->put(this, s3, num);
	t = this->op->get(this, s3);
	printf("s3 = %d size = %zu\n", *t, this->op->get_size(this));

	sds s4 = sds_new("4567");
	num = zmalloc(sizeof(int));
	*num = 4567;
	this->op->put(this, s4, num);
	t = this->op->get(this, s4);
	printf("s4 = %d size = %zu\n", *t, this->op->get_size(this));

	sds s5 = sds_new("5678");
	num = zmalloc(sizeof(int));
	*num = 5678;
	this->op->put(this, s5, num);
	t = this->op->get(this, s5);
	printf("s5 = %d size = %zu\n", *t, this->op->get_size(this));

	num = zmalloc(sizeof(int));
	*num = 1111;
	this->op->put(this, s, num);
	t = this->op->get(this, s);
	printf("s = %d size = %zu\n", *t, this->op->get_size(this));

	num = zmalloc(sizeof(int));
	*num = 2222;
	sds s6 = sds_new("6666");
	this->op->put(this, s6, num);
	t = this->op->get(this, s6);
	printf("s6 = %d size = %zu\n", *t, this->op->get_size(this));

	t = this->op->get(this, s2);
	if(t == 0) {
		puts("NONE S2");
	}else {
		printf("s2 = %d size = %zu\n", *t, this->op->get_size(this));
	}

	num = zmalloc(sizeof(int));
	*num = 3333;
	this->op->put(this, s2, num);
	t = this->op->get(this, s2);
	printf("s2 = %d size = %zu\n", *t, this->op->get_size(this));

	sds_free(s);
	sds_free(s2);
	sds_free(s3);
	sds_free(s4);
	sds_free(s5);
	sds_free(s6);
	destroy_lru_cache(this);
	return 0;

}
#endif
