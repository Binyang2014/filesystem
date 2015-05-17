#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "../threadpool.h"

/*=================================EVENT STUB=====================================*/

//1 for add
void add(event_handler_t* event_handler)
{
	printf("This is an add operation\n");
	sleep(1);
	printf("Add operation finished\n");
}

//2 for sub
void sub(event_handler_t* event_handler)
{
	printf("This is a sub operation\n");
	sleep(1);
	printf("Sub operation finished\n");
}

//It't a simple resolve and 1 for add 2 for sub
void* resolve_handler(event_handler_t* event_handler, void* msg_queue)
{
	common_msg_t common_msg;
	basic_queue_t* t_msg_queue = msg_queue;
	t_msg_queue->basic_queue_op->pop(t_msg_queue, &common_msg);
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
	basic_queue_t* msg_queue;
	thread_pool_t* thread_pool;
	event_handler_set_t* event_handler;
	common_msg_t common_msg;
	int i;

	msg_queue = alloc_msg_queue();
	thread_pool = alloc_thread_pool(8, msg_queue);
	event_handler = alloc_event_handler_set(thread_pool, resolve_handler);

	thread_pool->tp_ops->start(thread_pool, event_handler);
	for(i = 0; i < 30; i++)
	{
		common_msg.operation_code = 1;
		msg_queue->basic_queue_op->push(msg_queue, &common_msg);

		common_msg.operation_code = 2;
		msg_queue->basic_queue_op->push(msg_queue, &common_msg);
	}

	distroy_thread_pool(thread_pool);
	destroy_event_handler_set(event_handler);
	destroy_msg_queue(msg_queue);
	return 0;
}
