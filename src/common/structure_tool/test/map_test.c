#include <stdio.h>
#include "../map.h"
#include "../sds.h"
#include "../zmalloc.h"

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
void *v_dup(const void *v){
	int *t = zmalloc(sizeof(int));
	*t = *((int *)v);
	return (void *)t;
}

int main() {
	//if v_dup or v_free is NULL, it will be shallow copy, make your copy function right
	map_t *map = create_map(10, v_dup, v_free, pair_dup, pair_free);

	//put
	sds s = sds_new("1234");
	int v = 1234;
	map->op->put(map, s, &v);
	int *t = map->op->get(map, s);
	printf("the value we get = %d\n", *t);
	zfree(t);

	//modify
	v = 2345;
	map->op->put(map, s, &v);
	t = map->op->get(map, s);
	printf("the value we get = %d\n", *t);
	zfree(t);

	//delete
	map->op->del(map, s);
	t = map->op->get(map, s);
	if(t == NULL) {
		puts("NONE");
	}else {
		printf("the value we get = %d\n", *t);
		zfree(t);
	}
	sds_free(s);

	destroy_map(map);
	return 0;
}
