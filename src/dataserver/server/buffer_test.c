#include <stdio.h>
#include <pthread.h>
#include "dataserver_buff.h"

void* thread_push(void* msg_queue)
{
	int i, j;
	common_msg_t common_msg;
	msg_queue_t* t_msg_queue = msg_queue;
	for(i = 55, j = 21; i < 60; i++, j++)
	{
		common_msg.operation_code = i;
		common_msg.source = j;
		printf("push a common message with code %d, source %d\n",
				common_msg.operation_code, common_msg.source);
		t_msg_queue->msg_op->push(t_msg_queue, &common_msg);
	}
	return NULL;
}

void* thread_pop(void* msg_queue)
{
	int i;
	common_msg_t common_msg;
	msg_queue_t* t_msg_queue = msg_queue;
	for(i = 0; i < 5; i++)
	{
		t_msg_queue->msg_op->pop(t_msg_queue, &common_msg);
		printf("pop a common message with code %d, source %d\n",
				common_msg.operation_code, common_msg.source);
	}
	return NULL;
}

int main()
{
	msg_queue_t* msg_queue_test;
	pthread_t tid[2];

	msg_queue_test = alloc_msg_queue();

	pthread_create(&tid[0], NULL, thread_push, msg_queue_test);
	pthread_create(&tid[1], NULL, thread_pop, msg_queue_test);

	pthread_join(tid[0], NULL);
	pthread_join(tid[1], NULL);
	destroy_msg_queue(msg_queue_test);
	return 0;
}
