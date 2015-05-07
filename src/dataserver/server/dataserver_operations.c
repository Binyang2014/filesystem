/**
 * create on 2015.4.15
 * author: Binyang
 *
 * this file will finish data erver operations and I hope operatins will do not
 * care about buffer operations
 */

#include <stdlib.h>
#include "dataserver.h"

//receive cmd message from client or master
void* m_cmd_receive(void* msg_queue_arg)
{
	void* start_pos;
	common_msg_t* t_common_msg;
	msg_queue_t * msg_queue;
	mpi_status_t status;

	msg_queue = (msg_queue_t* )msg_queue_arg;
	t_common_msg = (common_msg_t* )malloc(sizeof(common_msg_t));

	while (1)
	{

#ifdef DATASERVER_COMM_DEBUG
		printf("The tid of this thread is %lu\n", (unsigned long)pthread_self());
		printf("I'm waiting for a message\n");
#endif
		//receive message from client
		start_pos = (char*) t_common_msg + COMMON_MSG_HEAD;
		d_mpi_cmd_recv(start_pos, &status);
		t_common_msg->source = status.source;
		//push message to the queue
		msg_queue->msg_op->push(msg_queue, t_common_msg);
	}

	free(t_common_msg);
	return NULL;
}
//------------------------------------------------------------------------------

static int resolve_msg(common_msg_t* common_msg)
{
	unsigned short operation_code;
	operation_code = common_msg->operation_code;

#ifdef DATASERVER_COMM_DEBUG
			printf("in resolve_msg and the operation code is %d\n", operation_code);
#endif

	switch(operation_code)
	{
	case MSG_READ:
		//invoke a thread to excuse
		d_read_handler(data_server, common_msg);
		break;
	case MSG_WRITE:
		//invoke a thread to excuse
		d_write_handler(data_server, common_msg);
		break;
	default:
		break;
	}
	return -1;
}

//resolve message from message queue
void m_resolve(msg_queue_t * msg_queue)
{
	common_msg_t* t_common_msg;

#ifdef DATASERVER_COMM_DEBUG
	printf("In m_resolve function\n");
#endif

	t_common_msg = (common_msg_t* )malloc(sizeof(common_msg_t));
	while(1)
	{
		msg_queue->msg_op->pop(msg_queue, t_common_msg);
		resolve_msg(t_common_msg);
	}
	free(t_common_msg);
}

