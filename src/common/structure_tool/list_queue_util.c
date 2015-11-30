/*
 * list_queue_util.c
 *
 *  Created on: 2015年10月12日
 *      Author: ron
 */
#include "list_queue_util.h"
#include "zmalloc.h"
#include <string.h>

/*
 * size
 * offset
 * write_length
 * data_server_num
 * position_des_t *;
 */
void *list_to_array(list_t *list, uint64_t *size, uint64_t offset, uint64_t write_len) {
	uint64_t length = (*size) * list->len + sizeof(uint64_t) * 4;
	position_des_t *pos = zmalloc(length);
	int index = 0;
	uint64_t *head = (uint64_t *)pos;
	*head = length;
	*(head + 1) = offset;
	*(head + 2) = write_len;
	*(head + 3) =  list->len;
	position_des_t *position = (position_des_t *)(head + 4);
	list_iter_t *iter = list->list_ops->list_get_iterator(list, AL_START_HEAD);
	while (list->list_ops->list_has_next(iter)) {
		position_des_t *p = ((list_node_t *) list->list_ops->list_next(iter))->value;
		memcpy(position + index, p, *size);
		index++;
	}
	list->list_ops->list_release_iterator(iter);
	*size = length;
	return pos;
}
//
//basic_queue_t *list_to_queue(list_t *list, size_t size) {
//	if (list == NULL || size <= 0) {
//		return NULL;
//	}
//
//	basic_queue_t *queue = alloc_basic_queue(list->len, size);
//
//	queue->dup = list->dup;
//	queue->free = list->free;
//	list_iter_t *iter = list->list_ops->list_get_iterator(list, AL_START_HEAD);
//	while (list->list_ops->list_has_next(iter)) {
//		queue->basic_queue_op->push(queue, (list_node_t*) list->list_ops->list_next(iter)->value);
//	}
//
//	return queue;
//}
