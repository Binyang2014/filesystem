#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "threadpool.h"

/*=============================MESSAGE QUEUE TEST===================================*/
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

//int main()
//{
//	msg_queue_t* msg_queue_test;
//	pthread_t tid[2];
//
//	msg_queue_test = alloc_msg_queue();
//
//	pthread_create(&tid[0], NULL, thread_push, msg_queue_test);
//	pthread_create(&tid[1], NULL, thread_pop, msg_queue_test);
//
//	pthread_join(tid[0], NULL);
//	pthread_join(tid[1], NULL);
//	destroy_msg_queue(msg_queue_test);
//	return 0;
//}

/*=================================EVENT STUB=====================================*/

//1 for add
void add(event_handler_t* event_handler)
{
	printf("This is an add operation\n");
	sleep(1);
}

//2 for sub
void sub(event_handler_t* event_handler)
{
	printf("This is a sub operation\n");
	sleep(1);
}

//It't a simple resolve and 1 for add 2 for sub
void* resolve_handler(event_handler_t* event_handler, void* msg_queue)
{
	common_msg_t common_msg;
	msg_queue_t* t_msg_queue = msg_queue;
	t_msg_queue->msg_op->pop(t_msg_queue, &common_msg);
	switch(common_msg.operation_code)
	{
	case 1:
		event_handler->handler = add;
		break;
	case 2:
		event_handler->handler = sub;
		break;
	default:
		event_handler->handler = NULL;
		break;

	}
	return event_handler->handler;
}


/*================================MAIN FUNCTION====================================*/
int main()
{
	msg_queue_t* msg_queue;
	thread_pool_t* thread_pool;
	event_handler_set_t* event_handler;
	common_msg_t common_msg;

	msg_queue = alloc_msg_queue();
	thread_pool = alloc_thread_pool(3, msg_queue);
	event_handler = alloc_event_handler_set(thread_pool, resolve_handler, 3);

	common_msg.operation_code = 1;
	msg_queue->msg_op->push(msg_queue, &common_msg);

	common_msg.operation_code = 2;
	msg_queue->msg_op->push(msg_queue, &common_msg);

	thread_pool->tp_ops->start(thread_pool, event_handler);

	//while(1);
	sleep(10);
	distroy_thread_pool(thread_pool);
	destroy_event_handler_set(event_handler);
	destroy_msg_queue(msg_queue);
	return 0;
}
