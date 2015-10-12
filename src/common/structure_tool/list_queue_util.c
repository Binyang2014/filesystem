/*
 * list_queue_util.c
 *
 *  Created on: 2015年10月12日
 *      Author: ron
 */


void *list_to_array(list_t *list, int size){
	position_des_t *pos = zmalloc(size * list->len);
	int index = 0;
	list_iter_t *iter = list->list_ops->list_get_iterator(list);
	while(list->list_ops->list_has_next(iter)){
		position_des_t *p = ((list_node_t *)list->list_ops->list_next(iter))->value;
		memcpy(pos + index, p, size);
		index++;
	}
	list->list_ops->list_release_iterator(iter);
	return pos;
}
