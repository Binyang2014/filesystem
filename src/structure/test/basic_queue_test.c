/*
 * basic_queue_test.c
 *
 *  Created on: 2015年5月15日
 *      Author: ron
 */
#include "../basic_queue.h"
#include "../../tool/message.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

pthread_t *consumer1;
pthread_t *consumer2;
pthread_t *producer;
pthread_mutex_t *mutex_queue;
pthread_cond_t *cond_empty;
basic_queue_t *queue;

char *buff, *msg_buff, *receive_buf;
common_msg_t *msg, *pop_msg;
static void set_common_msg(common_msg_t *msg, int source, char *message){
	unsigned short *operation_code = (unsigned short*)message;
	msg->operation_code = *operation_code;
	msg->source = source;
	memcpy(msg->rest, message, 4096);
	//client_create_file *file_request = msg->rest;
	//err_ret("set_common_msg: file_name = %s", file_request->file_name);
}

static int queue_push_msg(basic_queue_t *queue, int source, char *message){
	set_common_msg(msg, source, receive_buf);
	queue->basic_queue_op->push(queue, msg);
	return 999;
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

/*
void test_create_basic_list(){
	basic_queue_t *queue = alloc_basic_queue(sizeof(int), 4);
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
}*/

void *producer_product(){
	int x;
	int count = 10;
	//sleep(3);
	client_create_file *ccf = (client_create_file *)malloc(sizeof(client_create_file));
	ccf->operation_code = 100;
	ccf->file_size = 1000;
	memcpy(receive_buf, ccf, sizeof(client_create_file));
	while(1){
		//puts("put Start");
		pthread_mutex_lock(mutex_queue);
		queue_push_msg(queue, 1, receive_buf);
		pthread_mutex_unlock(mutex_queue);
		pthread_cond_signal(cond_empty);
		puts("put OK");
	}
	return 0;
}

void *consumer_product(){
	//int x;
	int count = 10;
	sleep(2);
	while(1){
		//puts("Get Start");
		pthread_mutex_lock(mutex_queue);
		while(queue->basic_queue_op->is_empty(queue)){
			//puts("queue empty");
			pthread_cond_wait(cond_empty, mutex_queue);
		}
		printf("queue size = %d\n", queue->current_size);
		queue->basic_queue_op->pop(queue, pop_msg);
		client_create_file *ccf = pop_msg->rest;
		printf("msg val = %ld\n", ccf->file_size);
		pthread_mutex_unlock(mutex_queue);
		puts("get OK");
	}
	return 0;
}

void test_producer_consumer(){
	queue = alloc_basic_queue(sizeof(common_msg_t), 4);
	queue_set_dup(queue, common_msg_dup);
	receive_buf = (char *)malloc(4096);
	msg_buff = (char *)malloc(4096);
	msg = (common_msg_t *)malloc(sizeof(common_msg_t));
	pop_msg = (common_msg_t *)malloc(sizeof(common_msg_t));
	consumer1 = (pthread_t *)malloc(sizeof(pthread_t));
	consumer2 = (pthread_t *)malloc(sizeof(pthread_t));
	producer =  (pthread_t *)malloc(sizeof(pthread_t));
	mutex_queue = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
	cond_empty = (pthread_cond_t *)malloc(sizeof(pthread_cond_t));
	pthread_mutex_init(mutex_queue, NULL);
	pthread_cond_init(cond_empty, NULL);
	//pthread_create(consumer1, NULL, consumer_product, NULL);
	pthread_create(consumer2, NULL, consumer_product, NULL);
	pthread_create(producer, NULL, producer_product, NULL);
	//pthread_join(*consumer1, NULL);
	pthread_join(*consumer2, NULL);
	pthread_join(*producer, NULL);
}

int main(){
	//test_create_basic_list();
	test_producer_consumer();
	return 0;
}

