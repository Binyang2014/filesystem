/**
 * created on 2015.4.16
 * author: Binyang
 * buffer handler functions
 */
#include <string.h>
#include <stdlib.h>
#include "dataserver_buff.h"
#include "../../tool/errinfo.h"

//message queue operation functions
void msg_queue_push(msg_queue_t* this, common_msg_t* msg )
{
	int offset;

	sem_wait(this->msg_queue_empty);
	offset = this->tail_pos;
	memcpy(this->msg + offset, msg, sizeof(common_msg_t));
	this->tail_pos = this->tail_pos + 1;
	sem_post(this->msg_queue_full);
}

void msg_queue_pop(msg_queue_t* this, common_msg_t* msg)
{
	int offset;

	sem_wait(this->msg_queue_full);
	offset = this->head_pos;
	memcpy(msg, this->msg + offset, sizeof(common_msg_t));
	this->head_pos = this->head_pos + 1;
	sem_post(this->msg_queue_empty);
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
	this->msg_queue_empty = (sem_t* )malloc(sizeof(sem_t));
	if(this->msg_queue_empty == NULL)
	{
		free(this->msg);
		free(this);
		return NULL;
	}
	this->msg_queue_full = (sem_t* )malloc(sizeof(sem_t));
	if(this->msg_queue_full == NULL)
	{
		free(this->msg_queue_empty);
		free(this->msg);
		free(this);
		return NULL;
	}
	sem_init(this->msg_queue_empty, 0, D_MSG_BSIZE);
	sem_init(this->msg_queue_full, 0, 0);

	this->msg_op = (msg_queue_op_t* )malloc(sizeof(msg_queue_op_t));
	if(this->msg_op == NULL)
	{
		free(this->msg_queue_full);
		free(this->msg_queue_empty);
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
	sem_destroy(this->msg_queue_empty);
	sem_destroy(this->msg_queue_full);
	free(this->msg_op);
	free(this->msg_queue_empty);
	free(this->msg_queue_full);
	free(this->msg);
	free(this);
}
