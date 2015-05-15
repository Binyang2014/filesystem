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
	basic_queue_t *queue = alloc_msg_queue(sizeof(int), 4);
	queue_set_free(queue, free_val);
	queue_set_dup(queue, dup);
	int x = 10008;
	int i;
	for(i = 0; i != 100; i++){
		test_queue_push(queue, &x);
	}
	i = 0;
	printf("current_size = %d\n", queue->current_size);
	int size =  queue->current_size;
	for(i = 0; i != size + 1; i++){
		queue->basic_queue_op->pop(queue, &x);
		printf("current_size = %d i = %d\n %d", queue->current_size, i, x);
	}
	print_queue(queue);
}

int main(){
	test_create_basic_list();
	return 0;
}

