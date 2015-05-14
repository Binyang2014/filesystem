#include "../global.h"
#include "../tool/message.h"

#define Q_FULL(head_pos, tail_pos) (((tail_pos + 1) % D_MSG_BSIZE) == (head_pos))
#define Q_EMPTY(head_pos, tail_pos) ((head_pos) == (tail_pos))

struct msg_queue_op
{
	void (*push)(struct msg_queue* , common_msg_t* );
	void (*pop)(struct msg_queue* , common_msg_t* );
	int  (*is_empty)(struct msg_queue*);
	int  (*is_full)(struct msg_queue*);
};

/*
 * this msg_queue do not provide mutex lock
 * if multiple threads will modify it, you should
 * lock the mutex by yourself
 */
struct msg_queue
{
	//int current_size;
	int head_pos;
	int tail_pos;
	int msg_count;
	int queue_len;
	int unit_size;
	struct msg_queue_op* msg_op;
	common_msg_t* msg;
};

typedef struct msg_queue msg_queue_t;
typedef struct msg_queue_op msg_queue_op_t;

//you also can use pop and push function in message queue
msg_queue_t* alloc_msg_queue();
void destroy_msg_queue(msg_queue_t* );
