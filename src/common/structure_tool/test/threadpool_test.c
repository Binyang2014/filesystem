#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdint.h>
#include "../syn_tool.h"
#include "../threadpool.h"
#include "../log.h"

syn_queue_t* queue_syn;
typedef struct common_msg
{
	int16_t operation_code;
}common_msg_t;

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
	syn_queue_t* queue = msg_queue;

	queue->op->syn_queue_pop(queue_syn, &common_msg);
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
	thread_pool_t* thread_pool;
	common_msg_t common_msg;
	//syn_queue_t* queue_syn;
	int i;

	//log_init("/home/binyang/Program/filesystem/src/common/test", LOG_DEBUG);
	log_init("", LOG_DEBUG);
	queue_syn = alloc_syn_queue(10, sizeof(common_msg_t));
	thread_pool = alloc_thread_pool(8, queue_syn, resolve_handler);

	thread_pool->tp_ops->start(thread_pool);
	for(i = 0; i < 30; i++)
	{
		common_msg.operation_code = 1;
		queue_syn->op->syn_queue_push(queue_syn, &common_msg);

		common_msg.operation_code = 2;
		queue_syn->op->syn_queue_push(queue_syn, &common_msg);
	}

	destroy_thread_pool(thread_pool);
	destroy_syn_queue(queue_syn);
	log_close();
	return 0;
}
