#include <string.h>
#include "message_queue.h"
#include "../global.h"

static void msg_queue_push(msg_queue_t*, common_msg_t*);
static void msg_queue_pop(msg_queue_t*, common_msg_t*);
static int is_empty(msg_queue_t*);
static int is_full(msg_queue_t*);

/*==============================PRIVATE VARIABLES========================*/
static const int default_capacity = 1 << 8;
static const double load_factor = 0.75;

/*==============================PRIVATE PROTOTYPES=======================*/
static int cal_new_length(int length);
static int mes_queue_resize(const msg_queue_t* this);
static void copy_queue_mes(msg_queue_t* des, msg_queue_t* source, int source_start, int source_end, int old_length);

/*==============================MESSAGE QUEUE=============================*/
//calculate the new queue length for resizing
static int cal_new_length(int length) {
	int new_length = 1;
	while (new_length <= length) {
		//TODO don't forget to check here
		new_length = new_length << 1;
	}
	return new_length << 1;
}

static int mes_queue_resize(const msg_queue_t* this) {
	int new_length = cal_new_length(this->queue_len);
	common_msg_t *new_com_msg_t = (common_msg_t*) malloc(
			sizeof(common_msg_t) * new_length);
	if (new_com_msg_t == NULL)
		return -1;

	copy_queue_mes(new_com_msg_t, this->msg, this->head_pos, this->tail_pos);
	this->head_pos = 0;
	this->tail_pos = this->msg_count;
	free(this->msg);
	this->msg = new_com_msg_t;
	this->queue_len = new_length;
	new_com_msg_t = NULL;
	//TODO if new_com_msg_t is NULL, there must not be enough space and we should handle this situation
	return 1;
}

static void copy_queue_mes(common_msg_t* des, common_msg_t* source, int start,
		int end, int old_length) {
	if (end > start) {
		memcpy(des, source + start, end - start);
	} else {
		memcpy(des, source + start, old_length - start);
		memcpy(des + old_length - start, source, end + 1);
	}
}

static void msg_queue_push(msg_queue_t* this, common_msg_t* msg) {
	assert(this->msg_count != this->queue_len);

	int offset = this->tail_pos;
	memcpy(this->msg + offset, msg, sizeof(common_msg_t));
	this->tail_pos = (this->tail_pos + 1) % this->queue_len;
	this->msg_count++;
	if (load_factor * this->queue_len > this->msg_count) {
		if(mes_queue_resize(this) == -1){
			err_ret("there is not enough space to resize the queue");
		}
	}
}

static void msg_queue_pop(msg_queue_t* this, common_msg_t* msg) {
	if (is_empty(this))
		msg = NULL;
	int offset = this->head_pos;
	memcpy(msg, this->msg + offset, sizeof(common_msg_t));
	this->head_pos = (this->head_pos + 1) % this->queue_len;
	this->msg_count--;
}

static int is_empty(msg_queue_t* this) {
	return this->msg_count == 0;
}

static int is_full(msg_queue_t* this){
	return this->queue_len == this->msg_count;
}

msg_queue_t* alloc_msg_queue() {
	msg_queue_t* this;
	this = (msg_queue_t*) malloc(sizeof(msg_queue_t));
	if (this == NULL) {
		err_ret("error in alloc_msg_queue");
		return NULL;
	}

	this->head_pos = 0;
	this->tail_pos = 0;
	this->msg_count = 0;
	this->queue_len = default_capacity;
	//this->current_size = 0;
	this->msg = (common_msg_t*) malloc(sizeof(common_msg_t) * D_MSG_BSIZE);
	if (this->msg == NULL) {
		free(this);
		return NULL;
	}

	//init message operations
	this->msg_op = (msg_queue_op_t*) malloc(sizeof(msg_queue_op_t));
	if (this->msg_op == NULL) {
		free(this->msg);
		free(this);
		return NULL;
	}

	this->msg_op->push = msg_queue_push;
	this->msg_op->pop = msg_queue_pop;
	this->msg_op->is_empty = is_empty;
	return this;
}

void destroy_msg_queue(msg_queue_t* this) {
	this->msg_op->push = NULL;
	;
	this->msg_op->pop = NULL;
	free(this->msg_op);
	free(this->msg);
	free(this);
}
