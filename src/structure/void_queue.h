typedef struct void_queue_op
{
	void (*push)(struct msg_queue* , void* );
	void (*pop)(struct msg_queue* , void* );
	int  (*is_empty)(struct msg_queue*);
	int  (*is_full)(struct msg_queue*);
}void_queue_op;

/*
 * this msg_queue do not provide mutex lock
 * if multiple threads will modify it, you should
 * lock the mutex by yourself
 */
typedef struct void_queue
{
	//int current_size;
	int head_pos;
	int tail_pos;
	int current_size;
	int queue_len;
	int void_size;
	void_queue_op* void_queue_op;
	void* value;
}void_queue;


//you also can use pop and push function in message queue
void_queue* create_void_queue(int);
void destroy_void_queue(void_queue* );
