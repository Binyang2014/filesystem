/*
 * map.c
 *
 *  Created on: 2015年7月2日
 *      Author: ron
 */

#include <stdio.h>
#include <assert.h>
#include "map.h"
#include "zmalloc.h"


/* Copy from redis
 * MurmurHash2, by Austin Appleby
 * Note - This code makes a few assumptions about how your machine behaves -
 * 1. We can read a 4-byte value from any address without crashing
 * 2. sizeof(int) == 4
 *
 * And it has a few limitations -
 *
 * 1. It will not work incrementally.
 * 2. It will not produce the same results on little-endian and big-endian
 *    machines.
 */
static uint32_t map_gen_hash_function(const sds key, size_t size) {
	size_t len = sds_len(key);
	/* 'm' and 'r' are mixing constants generated offline.
	   They're not really 'magic', they just happen to work well.  */
	uint32_t seed = 5381;
	const uint32_t m = 0x5bd1e995;
	const int r = 24;

	/* Initialize the hash to a 'random' value */
	uint32_t h = seed ^ len;

	/* Mix 4 bytes at a time into the hash */
	const unsigned char *data = (const unsigned char *)key;
	while(len >= 4) {
		uint32_t k = *(uint32_t*)data;
		k *= m;
		k ^= k >> r;
		k *= m;

		h *= m;
		h ^= k;

		data += 4;
		len -= 4;
	}
	/* Handle the last few bytes of the input array  */
	switch(len) {
		case 3: h ^= data[2] << 16;
				h ^= data[1] << 8;
				h ^= data[0]; h *= m;
				break;
		case 2: h ^= data[1] << 8;
				h ^= data[0]; h *= m;
				break;
		case 1: h ^= data[0]; h *= m;
	};

	/* Do a few final mixes of the hash to ensure the last few
	 * bytes are well-incorporated. */
	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;

	return (uint32_t)h % size;
}

static int key_match(void* ptr, void* key)
{
	sds to_be_matched = ptr;
	sds searched = key;

	return !(sds_cmp(to_be_matched, searched));
}

static pair_t *find(map_t *this, const sds key){
	uint32_t h = map_gen_hash_function(key, this->size);

	list_t *l = *(this->list + h);

	list_iter_t *iter = l->list_ops->list_get_iterator(l, AL_START_HEAD);
	list_node_t *node = l->list_ops->list_next(iter);
	while(node) {
		if(sds_cmp(((pair_t *)node->value)->key, key) == 0) {
			l->list_ops->list_release_iterator(iter);
			return (pair_t *)node->value;
		}
		node = l->list_ops->list_next(iter);
	}
	l->list_ops->list_release_iterator(iter);

	return NULL;
}

static int put(map_t *this, sds key, void *value) {
	pair_t *node = find(this, key);

	if(node == NULL) {
		uint32_t h = map_gen_hash_function(key, this->size);
		list_t *l = *(this->list + h);

		pair_t *pair = (pair_t *)zmalloc(sizeof(pair_t));
		pair->key = sds_dup(key);
		pair->value = this->value_dup ? this->value_dup(value) : value;

		l->list_ops->list_add_node_tail(l, (void *)pair);
		this->current_size++;
		//change key_list
		this->key_list->list_ops->list_add_node_tail(this->key_list,
				sds_dup(pair->key));
		return 1;
	}else {
		if(this->value_free) {
			this->value_free(node->value);
		}else {
			zfree(node->value);
		}
		node->value = this->value_dup ? this->value_dup(value) : value;
		return 0;
	}
}

static void *get(map_t *this, const sds key) {
	pair_t *node = find(this, key);

	return node == NULL ? NULL : (this->value_dup ? this->value_dup(node->value) : node->value);
}

static int contains(map_t *this, sds key) {
	return find(this, key) != NULL;
}

static int modify_key(map_t *this, sds old_key, sds new_key) {
	uint32_t h = map_gen_hash_function(old_key, this->size);

	list_t *l = *(this->list + h);
	list_iter_t *iter = l->list_ops->list_get_iterator(l, AL_START_HEAD);
	list_node_t *node = l->list_ops->list_next(iter);
	list_node_t *key_list_node;
	while(node) {
		if(sds_cmp(((pair_t *)node->value)->key, old_key) == 0) {
			l->list_ops->list_remove_node(l, node);
			sds_free(((pair_t *)node->value)->key);
			put(this, sds_dup(new_key), node->value);

			//change key_list
			this->key_list->list_ops->list_add_node_tail(this->key_list,
					sds_dup(new_key));
			key_list_node = this->key_list->list_ops->list_search_key(this->key_list,
						old_key);
			this->key_list->list_ops->list_del_node(this->key_list,
					key_list_node);

			l->list_ops->list_release_iterator(iter);
			if(this->value_dup) {
				this->key_free(((pair_t *)node->value)->key);
				this->value_free(((pair_t *)node->value)->value);
			}
			zfree(node);

			return 0;
		}
		node = l->list_ops->list_next(iter);
	}
	l->list_ops->list_release_iterator(iter);

	return -1;
}

static size_t get_size(map_t *this) {
	assert(this->current_size >= 0);

	return this->current_size;
}

static void del(map_t *this, sds key){
	uint32_t h = map_gen_hash_function(key, this->size);

	list_t *l = *(this->list + h);
	list_iter_t *iter = l->list_ops->list_get_iterator(l, AL_START_HEAD);
	list_node_t *node = l->list_ops->list_next(iter);
	list_node_t *key_list_node;
	while(node) {
		if(sds_cmp(((pair_t *)node->value)->key, key) == 0) {
			l->list_ops->list_del_node(l, node);
			this->current_size--;
			l->list_ops->list_release_iterator(iter);
			//change key_list
			key_list_node = this->key_list->list_ops->list_search_key(this->key_list,
					key);
			this->key_list->list_ops->list_del_node(this->key_list, key_list_node);
			return;
		}
		node = l->list_ops->list_next(iter);
	}
	l->list_ops->list_release_iterator(iter);
}

/* This function will look up keys in the map and put ervery key into a array.
 * The return value is a array of sds, user should realse this array by hand by
 * calling sds_free_split_res method or you can write free mothod by yourself.*/
static sds *get_all_keys(map_t *this, int *count)
{
	list_iter_t *iter;
	list_node_t *node;
	int index = 0;
	sds *array;
	list_t *key_list = this->key_list;

	*count = this->current_size;
	if(*count == 0)
		return NULL;
	array = zmalloc(sizeof(sds) * this->current_size);
	iter = key_list->list_ops->list_get_iterator(key_list, AL_START_HEAD);
	while((node = key_list->list_ops->list_next(iter)) != NULL)
		array[index++] = sds_dup((sds)(node->value));
	key_list->list_ops->list_release_iterator(iter);
	return array;
}

static int has_next(struct map_iterator *iterator)
{
	int offset = iterator->dir_no;
	for(; offset < iterator->map->size; offset++){
		if(iterator->list == NULL){
			if(*(iterator->map->list + offset) != NULL){
				iterator->list = *(iterator->map->list + offset);
				iterator->iter = iterator->list->list_ops->list_get_iterator(iterator->list, AL_START_HEAD);
			}else{
				continue;
			}
		}
		if(iterator->list->list_ops->list_has_next(iterator->iter)){
			return 1;
		}else{
			iterator->list->list_ops->list_release_iterator(iterator->iter);
			iterator->list = NULL;
		}
	}

	return 0;
}

static void *next(struct map_iterator *iterator){
	return ((list_node_t *)list_next(iterator->iter))->value;
//=======
//	return ((list_node_t *)iterator->list->list_ops->list_next(iterator->iter))->next;
//>>>>>>> origin/new
}

/* This function will create a map. In the construre, you should provide map
 * size: the map will have a fixed size. Besides, you should provide four other
 * functions, to indicate how to dup and free values and pairs. If you use NULL,
 * dup will simply as assign and free will use system free method if you wish.*/
map_t *create_map(size_t size, void *(*value_dup)(const void *value), void (*value_free)(void *value),
		void *(*list_dup)(const void *ptr), void (*list_free)(void *ptr)) {
	int i;

	map_t *this = (map_t *)(zmalloc(sizeof(map_t)));
	this->op = zmalloc(sizeof(map_op_t));
	this->list = (list_t **)zmalloc(sizeof(list_t *) * size);
	this->key_list = list_create();
	list_set_free_method(this->key_list, (void (*)(void*))sds_free);
	list_set_match_method(this->key_list, key_match);

	for(i = 0; i < size; i++) {
		*(this->list + i) = list_create();
		(*(this->list + i))->dup = list_dup;
		(*(this->list + i))->free = list_free;
	}

	this->current_size = 0;
	this->size = size;
	this->op->put = put;
	this->op->get = get;
	this->op->del = del;
	this->op->modify_key = modify_key;
	this->op->contains = contains;
	this->op->get_size = get_size;
	this->op->get_all_keys = get_all_keys;

	this->key_dup = sds_dup;
	this->key_free = sds_free;
	this->value_dup = value_dup;
	this->value_free = value_free;

	return this;
}

void destroy_map(map_t *this) {
	zfree(this->op);
	int i;
	for(i = 0; i < this->size; i++) {
		list_release(*(this->list + i));
	}
	list_release(this->key_list);
	zfree(this->list);
	zfree(this);
}

map_iterator_t *create_map_iterator(map_t *map) {
	map_iterator_t *this = zmalloc(sizeof(*this));
	this->map = map;
	this->dir_no = 0;
	this->list = *(map->list + this->dir_no);
	this->iter = this->list->list_ops->list_get_iterator(this->list,
			AL_START_HEAD);
	this->op = zmalloc(sizeof(map_iterator_op_t));
	this->op->has_next = has_next;
	this->op->next = next;

	return this;
}


void destroy_map_iterator(map_iterator_t *iter) {
	zfree(iter->op);
	if(iter->iter) {
		iter->list->list_ops->list_release_iterator(iter->iter);
	}
	zfree(iter);
}

void print_map_keys() {

}

/*------------------T	E	S	T----------------*/
#define MAP_TEST 0
#if (GLOBAL_TEST) || (MAP_TEST)

void list_free(void *value) {
	pair_t *p = value;
	sds_free(p->key);
	zfree(p->value);
	zfree(p);
}

int main() {
	map_t *map = create_map(10, NULL, NULL, NULL, list_free);
	sds key1 = sds_new("1234");
	sds key2 = sds_new("2345");
	sds key3 = sds_new("3456");
	sds key4 = sds_new("5678");
	sds key5 = sds_new("6789");

	int *num = zmalloc(sizeof(int));
	*num = 1234;
	map->op->put(map, key1, num);

	num = zmalloc(sizeof(int));
	*num = 2345;
	map->op->put(map, key2, num);

	num = zmalloc(sizeof(int));
	*num = 3456;
	map->op->put(map, key3, num);

	num = zmalloc(sizeof(int));
	*num = 4567;
	map->op->put(map, key4, num);

	num = zmalloc(sizeof(int));
	*num = 5678;
	map->op->put(map, key5, num);

	map->op->del(map, key5);
	map->op->get(map, key5);
	map->op->del(map, key5);
	num = zmalloc(sizeof(int));
	*num = 5678;
	map->op->put(map, key5, num);

	sds_free(key1);
	sds_free(key2);
	sds_free(key3);
	sds_free(key4);
	sds_free(key5);
	destroy_map(map);
}
#endif

