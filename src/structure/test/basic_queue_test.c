/*
 * basic_queue_test.c
 *
 *  Created on: 2015年5月15日
 *      Author: ron
 */
#include "../basic_queue.h"
#include <stdlib.h>
#include <stdio.h>

void free_val(void *val){
	free(val);
}

void dup(void *dest, void *source){
	*(int *)dest = *(int *)source;
}

void print_queue(basic_queue_t *queue){
	puts("*****start print queue*****");
	printf("current_size = %d\nqueue_len = %d\n", queue->current_size, queue->queue_len);
	int offset = (queue->head_pos) * (queue->element_size);
	while(offset != queue->tail_pos * queue->element_size){
		printf("%d\n", *(int *)(queue->elements + offset));
		offset = (offset + queue->element_size) % (queue->queue_len * queue->element_size);
	}
	puts("*****end print queue*****");
}

void test_queue_push(basic_queue_t *queue, void *element){
	queue->basic_queue_op->push(queue, element);
	//print_queue(queue);
}

void test_create_basic_list(){
	basic_queue_t *queue = alloc_basic_queue(sizeof(int), 4);
	queue_set_free(queue, free_val);
	queue_set_dup(queue, dup);
	int x = 10008;
	int i;
	for(i = 0; i != 16; i++){
		test_queue_push(queue, &i);
		//printf("next = %d queue_len = %d head = %d tail = %d\n ", x, queue->queue_len, queue->head_pos, queue->tail_pos);
	}

	for(i = 0; i != 10; i++){
		queue->basic_queue_op->pop(queue, &x);
		//printf("next = %d queue_len = %d head = %d tail = %d\n ", x, queue->queue_len, queue->head_pos, queue->tail_pos);
	}
	for(i = 0; i != 5; i++){
			test_queue_push(queue, &i);
			//printf("next = %d queue_len = %d head = %d tail = %d\n ", x, queue->queue_len, queue->head_pos, queue->tail_pos);
	}
//	printf("current_size = %d\n", queue->current_size);
	basic_queue_iterator * iterator = create_basic_queue_iterator(queue);
	while(iterator->has_next(iterator)){
		iterator->next(iterator, &x);
		printf("next = %d queue_len = %d offset=%d\n ", x, iterator->queue->queue_len, iterator->offset);
	}
//	printf("current_size = %d\n", queue->current_size);
//	int size =  queue->current_size;
//	for(i = 0; i != size + 1; i++){
//		queue->basic_queue_op->pop(queue, &x);
//		printf("current_size = %d i = %d\n %d", queue->current_size, i, x);
//	}
	//print_queue(queue);
}

int main(){
	test_create_basic_list();
	return 0;
}

