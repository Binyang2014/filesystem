#include <pthread.h>
#include <stdio.h>
#include "../syn_tool.h"
#include "../message.h"
#include "../../structure/basic_queue.h"

queue_syn_t* queue_syn;

/*=============================MESSAGE QUEUE TEST===================================*/
void* thread_push(void* msg_queue)
{
	int i, j;
	common_msg_t common_msg;
	basic_queue_t* t_msg_queue = msg_queue;
	for(i = 55, j = 21; i < 60; i++, j++)
	{
		common_msg.operation_code = i;
		common_msg.source = j;
		printf("push a common message with code %d, source %d\n",
				common_msg.operation_code, common_msg.source);
		syn_queue_push(t_msg_queue, queue_syn, &common_msg);
	}
	return NULL;
}

void* thread_pop(void* msg_queue)
{
	int i;
	common_msg_t common_msg;
	basic_queue_t* t_msg_queue = msg_queue;
	for(i = 0; i < 5; i++)
	{
		syn_queue_pop(t_msg_queue, queue_syn, &common_msg);
		printf("pop a common message with code %d, source %d\n",
				common_msg.operation_code, common_msg.source);
	}
	return NULL;
}

int main()
{
	basic_queue_t* msg_queue_test;
	pthread_t tid[2];

	msg_queue_test = alloc_basic_queue(sizeof(common_msg_t), 2);
    queue_syn = alloc_queue_syn();

	pthread_create(&tid[0], NULL, thread_push, msg_queue_test);
	pthread_create(&tid[1], NULL, thread_pop, msg_queue_test);

	pthread_join(tid[0], NULL);
	pthread_join(tid[1], NULL);

	destroy_queue_syn(queue_syn);
	destroy_basic_queue(msg_queue_test);
	return 0;
}
