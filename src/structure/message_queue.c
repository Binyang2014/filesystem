#include <string.h>
#include "message_queue.h"
#include "../global.h"

static void msg_queue_push(msg_queue_t*, common_msg_t* );
static void msg_queue_pop(msg_queue_t* , common_msg_t* );

/*==============================MESSAGE QUEUE=============================*/
static void msg_queue_push(msg_queue_t* this, common_msg_t* msg )
{
	int offset;

	offset = this->tail_pos;
	memcpy(this->msg + offset, msg, sizeof(common_msg_t));
	this->tail_pos = (this->tail_pos + 1) % this->queue_len;
}

static void msg_queue_pop(msg_queue_t* this, common_msg_t* msg)
{
	int offset;

	offset = this->head_pos;
	memcpy(msg, this->msg + offset, sizeof(common_msg_t));
	this->head_pos = (this->head_pos + 1) % this->queue_len;
}

msg_queue_t* alloc_msg_queue()
{
	msg_queue_t* this;
	this = (msg_queue_t* )malloc(sizeof(msg_queue_t));
	if(this == NULL)
	{
		err_ret("error in alloc_msg_queue");
		return NULL;
	}

	this->head_pos = 0;
	this->tail_pos = 0;
	//this->current_size = 0;
	this->msg = (common_msg_t* )malloc(sizeof(common_msg_t) * D_MSG_BSIZE);
	if(this->msg == NULL)
	{
		free(this);
		return NULL;
	}

	//init message operations
	this->msg_op = (msg_queue_op_t* )malloc(sizeof(msg_queue_op_t));
	if(this->msg_op == NULL)
	{
		free(this->msg);
		free(this);
		return NULL;
	}

	this->msg_op->push = msg_queue_push;
	this->msg_op->pop = msg_queue_pop;
	return this;
}

void destroy_msg_queue(msg_queue_t* this)
{
	this->msg_op->push = NULL;;
	this->msg_op->pop = NULL;
	free(this->msg_op);
	free(this->msg);
	free(this);
}
