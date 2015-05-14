#include <assert.h>
#include <string.h>
#include "basic_queue.h"
#include "../global.h"

/*==============================PRIVATE VARIABLES========================*/
static const int default_capacity = 1 << 8;
//static const double load_factor = 0.75;

/*==============================PRIVATE PROTOTYPES=======================*/
static void basic_queue_push(basic_queue_t*, void*);
static void basic_queue_pop(basic_queue_t*, void*);
static int is_empty(msg_queue_t*);
static int is_full(msg_queue_t*);

/*==============================MESSAGE QUEUE=============================*/

static void basic_queue_push(basic_queue_t* this, void* element)
{
	int offset;
	assert(this->current_size != this->queue_len);

	offset = this->tail_pos;
	memcpy(this->elements + offset * this->element_size, element, this->element_size);
	this->tail_pos = (this->tail_pos + 1) % this->queue_len;
	this->current_size++;
	//I will not consider resize function for now
}

static void basic_queue_pop(basic_queue_t* this, void* element)
{
	int offset;
	if (is_empty(this))
	{
		element = NULL;
		return;
	}
	offset = this->head_pos;
	memcpy(element, this->elements + offset * this->element_size, this->element_size);
	this->head_pos = (this->head_pos + 1) % this->queue_len;
	this->current_size--;
}

static int is_empty(basic_queue_t* this)
{
	return this->current_size == 0;
}

static int is_full(basic_queue_t* this)
{
	return this->queue_len == this->current_size;
}

basic_queue_t* alloc_msg_queue(int type_size, int queue_len)
{
	basic_queue_t* this;
	this = (basic_queue_t*) malloc(sizeof(basic_queue_t));
	if (this == NULL) {
		err_ret("error in alloc_msg_queue");
		return NULL;
	}

	this->head_pos = 0;
	this->tail_pos = 0;
	this->current_size = 0;
	this->queue_len = queue_len > 0 ? queue_len : default_capacity;
	this->element_size = type_size;
	this->elements = (void*) malloc(type_size * this->queue_len);
	if (this->elements == NULL) {
		free(this);
		return NULL;
	}

	//initial queue operations
	this->basic_queue_op = (basic_queue_op_t*) malloc(sizeof(msg_queue_op_t));
	if (this->basic_queue_op == NULL) {
		free(this->elements);
		free(this);
		return NULL;
	}

	this->basic_queue_op->push = basic_queue_push;
	this->basic_queue_op->pop = basic_queue_pop;
	this->basic_queue_op->is_empty = is_empty;
	this->basic_queue_op->is_full = is_full;

	this->free = NULL;
	this->dup = NULL;
	return this;
}

void destroy_msg_queue(basic_queue_t* this)
{
	int i;
	this->basic_queue_op->push = NULL;
	this->basic_queue_op->pop = NULL;
	if(this->free)
	{
		for(i = 0; i < this->queue_len; i++)
			this->free(this->elements[i]);
	}
	free(this->elements);
	free(this->basic_queue_op);
	free(this);
}

