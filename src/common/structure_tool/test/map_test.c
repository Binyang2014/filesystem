#include <stdio.h>
#include "map.h"
#include "sds.h"
#include "zmalloc.h"

void *pair_dup(const void *pair){
	pair_t *p = zmalloc(sizeof(pair_t));
	p->key = sds_dup(((pair_t *)pair)->key);
	int *t = zmalloc(sizeof(int));
	*t = *((int *)((pair_t *)pair)->value);
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
	sds* array;
	int count, i;

	//put
	sds s = sds_new("1234");
	int v = 1234;
	map->op->put(map, s, &v);
	int *t = map->op->get(map, s);
	printf("the value we get = %d\n", *t);
	zfree(t);

	s = sds_cpy(s, "try again");
	v = 34;
	map->op->put(map, s, &v);

	//modify
	v = 2345;
	map->op->put(map, s, &v);
	t = map->op->get(map, s);
	printf("the value we get = %d\n", *t);
	zfree(t);

	array = map->op->get_all_keys(map, &count);
	printf("The key in the map is ");
	for(i = 0; i < count; i++)
		printf("%s ", array[i]);
	printf("\n");
	sds_free_split_res(array, count);

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
