/*
 * map.c
 *
 *  Created on: 2015年7月2日
 *      Author: ron
 */

#include "map.h"

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

/**
 * dup function for the list which resolve hash conflict
 */
static void *list_pair_dup(void *pair){
	pair_t *p;

	p = (pair_t *)zmalloc(sizeof(*p));
	p->key = ((pair_t *)pair)->key;
	p->value = ((pair_t *)pair)->value;

	return (void *)p;
}

static pair_t *find(map_t *this, const sds key){
	uint32_t h = map_gen_hash_function(key, this->size);

	//printf("hey hey this = %d\n", h);
	list_t *l = *(this->list + h);

	list_iter_t *iter = l->list_ops->list_get_iterator(l, AL_START_HEAD);
	list_node_t *node = l->list_ops->list_next(iter);
	while(node) {
		if(sds_cmp(((pair_t *)node->value)->key, key) == 0) {
			return (pair_t *)node->value;
		}
		node = l->list_ops->list_next(iter);
	}

	return NULL;
}

static int put(map_t *this, const sds key, const void *value) {
	pair_t *node = find(this, key);

	if(node == NULL) {
		uint32_t h = map_gen_hash_function(key, this->size);
		list_t *l = *(this->list + h);

		pair_t *pair = (pair_t *)zmalloc(sizeof(pair_t));
		pair->key = this->key_dup(key);
		pair->value = this->value_dup(value);

		l->list_ops->list_add_node_tail(l, (void *)pair);
		this->current_size++;
		return 1;
	}else {
		this->value_free(node->value);
		node->value = this->value_dup(value);
		return 0;
	}
}

static void *get(map_t *this, const sds key) {
	pair_t *node = find(this, key);

	return node == NULL ? NULL : this->value_dup(node->value);
}

static int contains(map_t *this, sds key) {
	return find(this, key) != NULL;
}

static size_t get_size(map_t *this) {
	return this->current_size;
}

void del(map_t *this, sds key){
	uint32_t h = map_gen_hash_function(key, this->size);

	list_t *l = *(this->list + h);
	list_iter_t *iter = l->list_ops->list_get_iterator(l, AL_START_HEAD);
	list_node_t *node = l->list_ops->list_next(iter);
	while(node) {
		if(sds_cmp(((pair_t *)node->value)->key, key) == 0) {
			l->list_ops->list_del_node(l, node);
			this->current_size--;
			return;
		}
		node = l->list_ops->list_next(iter);
	}
}

map_t *create_map(size_t size) {
	int i;

	map_t *this = (map_t *)(zmalloc(sizeof(map_t)));
	this->op = zmalloc(sizeof(map_op_t));
	this->list = (list_t **)zmalloc(sizeof(list_t *) * size);

	for(i = 0; i < size; i++) {
		*(this->list + i) = list_create();
		(*(this->list + i))->dup = list_pair_dup;
	}

	this->current_size = 0;
	this->size = size;
	this->op->put = put;
	this->op->get = get;
	this->op->contains = contains;
	this->op->get_size = get_size;

	this->key_dup = sds_dup;
	this->key_free = sds_free;

	return this;
}

void destroy_map(map_t *this) {
	zfree(this->op);
	int i;
	for(i = 0; i < this->size; i++) {
		list_release(*(this->list + i));
	}
	zfree(this);
}

#if 1
void *pair_dup(void *pair){
	pair_t *p = zmalloc(sizeof(pair_t));
	p->key = sds_dup(((pair_t *)pair)->key);
	int *t = zmalloc(sizeof(int));
	*t = *((int *)((pair_t *)pair)->key);
	p->value = t;
	return (void *)p;
}

void pair_free(void *pair){
	pair_t *p = pair;
	sds_free(p->key);
	zfree(p->value);
	zfree(p);
}

void v_free(void *v) {
	int *t = v;
	zfree(t);
}

void *v_dup(void *v){
	int *t = zmalloc(sizeof(int));
	*t = *((int *)v);
	return (void *)t;
}

int main() {
	map_t *map = create_map(10);
	set_map_value_dup(map, v_dup);
	set_map_value_free(map, v_free);
	set_list_pair_dup(map, pair_dup);
	set_list_pair_free(map, pair_free);

	sds s = sds_new("1234");
	int v = 1234;
	map->op->put(map, s, &v);
	int *t = map->op->get(map, s);
	printf("the value we get = %d\n", *t);
	v = 2345;
	map->op->put(map, s, &v);
	map->op->del(map, s);
	//t = map->op->get(map, s);

	//printf("the value we get = %d\n", *t);
	destroy_map(map);
	return 0;
}
#endif
