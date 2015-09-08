/**
 * created on 2015.4.28
 * author: Binyang
 * This is a long file
 */
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "zmalloc.h"
#include "threadpool.h"
#include "log.h"
#include "syn_tool.h"
#include "global.h"

static void* thread_do(void* arg);

static void init_thread(thread_pool_t* thread_pool, int id, event_handler_t* event_handler);
static pthread_cond_t* alloc_thread_cond(int threads_count);
static int push_stack(thread_pool_t* this, thread_t* thread);
static int pop_stack(thread_pool_t* this, thread_t* stack_top);
static int promote_a_leader(thread_pool_t* this, thread_t* thread);
static void deactive_handle(thread_pool_t* this);
static void reactive_handle(thread_pool_t* this);
static void start(thread_pool_t*);

static event_handler_t* alloc_event_handler(thread_pool_t*, resolve_handler_t);
static void destory_event_handler(event_handler_t*);
static int handle_event(thread_t* thread, event_handler_t* event_handler);

//there are handler and resolve_handler functions you should concern
//resolve function will resolve message and return handle function for this event
//the handler will do the right things for the event
static void destroy_event_handler_set(event_handler_set_t* event_handler_set);
static event_handler_set_t *alloc_event_handler_set(thread_pool_t* thread_pool,
		resolve_handler_t resolve_handler);

/*================================EVENT HANDLER=======================================*/

/*
 * @args:
 * event_handler_t* this
 * resolve message from message queue and assign right handler to
 * this events. promote a new leader, then do handler this event
 */
static int handle_event(thread_t* thread, event_handler_t* event_handler)
{
	handler_t handler;
#ifdef THREAD_POOL_DEBUG
	log_write(LOG_DEBUG, "Now the leader thread is %d", thread->id);
#endif
	event_handler->thread_pool->tp_ops->deactive_handle(event_handler->thread_pool);
	while(thread->canceled)
	{
		sleep(1);//wait cancel signal
		pthread_testcancel();
	}
	event_handler->thread_pool->tp_ops->promote_a_leader(event_handler->thread_pool, thread);

#ifdef THREAD_POOL_DEBUG
	log_write(LOG_DEBUG, "Now new leader has been selected");
#endif
	//Resolve handler for thread
	handler = event_handler->resolve_handler(event_handler, event_handler->thread_pool->msg_queue);
	//reduce spare threads count
	event_handler->thread_pool->spare_treads_count--;
	event_handler->thread_pool->tp_ops->reactive_handle(event_handler->thread_pool);
	event_handler->handler = handler;
#ifdef THREAD_POOL_DEBUG
	log_write(LOG_DEBUG, "thread %d is do handling", thread->id);
#endif
	event_handler->handler(event_handler);
	return 0;
}

/*
 * @args:
 * thread_pool: thread pool
 * resolve_handler function that will resolve message queue
 * @return:
 * event_hanlder_t* if allocated failed return null
 */
static event_handler_t* alloc_event_handler(thread_pool_t* thread_pool,
		resolve_handler_t resolve_handler)
{
	event_handler_t* event_handler = (event_handler_t* )zmalloc(sizeof(event_handler_t));
	if(event_handler == NULL)
	{
		err_ret("allocate event handler error");
		return NULL;
	}
	event_handler->event_buffer_list = NULL;
	event_handler->handler = NULL;
	event_handler->resolve_handler = resolve_handler;
	event_handler->thread_pool = thread_pool;
	return event_handler;
}

/*
 * @args:event_handler
 * release event handler
 */
static void destory_event_handler(event_handler_t* event_handler)
{
	event_handler->event_buffer_list = NULL;
	event_handler->handler = NULL;
	event_handler->resolve_handler = NULL;
	event_handler->thread_pool = NULL;
	zfree(event_handler);
}

//This function is used by threadpool alloc
static event_handler_set_t *alloc_event_handler_set(thread_pool_t* thread_pool,
		resolve_handler_t resolve_handler)
{
	int i, handlers_count;
	handlers_count = thread_pool->threads_count;
	event_handler_set_t* event_handler_set = (event_handler_set_t* )zmalloc
			(sizeof(event_handler_set_t));
	if(event_handler_set == NULL)
	{
		err_ret("allocate event handler set error");
		return NULL;
	}
	event_handler_set->event_handler_arr = (event_handler_t** )zmalloc
			(sizeof(event_handler_t*) * handlers_count);
	event_handler_set->event_handlers_conut = handlers_count;
	for(i = 0; i < handlers_count; i++)
	{
		event_handler_set->event_handler_arr[i] = alloc_event_handler(thread_pool, resolve_handler);
	}
	return event_handler_set;
}

static void destroy_event_handler_set(event_handler_set_t* event_handler_set)
{
	int i;
	int count = event_handler_set->event_handlers_conut;
	event_handler_t** event_handler_arr = event_handler_set->event_handler_arr;
	for(i = 0; i < count; i++)
	{
		destory_event_handler(event_handler_arr[i]);
	}
	zfree(event_handler_set->event_handler_arr);
	zfree(event_handler_set);
}

/*====================================THREAD==========================================*/

static void cleanup(void* arg)
{
	printf("cleanup: the thread number is %d\n", *(int*)arg);
}

static void* thread_do(void* arg)
{
	thread_t* this = (thread_t* )arg;
	int leader_id;
#ifdef THREAD_POOL_DEBUG
	log_write(LOG_DEBUG, "thread %d has been created!", this->id);
#endif
	pthread_cleanup_push(cleanup, &(this->id));
	pthread_mutex_lock(this->thread_pool->pool_mutex);
	for(;;)
	{
		leader_id = this->thread_pool->leader_id;
		if(leader_id != NO_THREAD_LEADER && this->id != leader_id)
		{
#ifdef THREAD_POOL_DEBUG
			log_write(LOG_DEBUG, "thread %d is going to wait signal", this->id);
#endif
			push_stack(this->thread_pool, this);
			pthread_cond_wait(this->thread_pool->pool_condition + this->id,
					this->thread_pool->pool_mutex);
			while(this->canceled)
			{
				pthread_mutex_unlock(this->thread_pool->pool_mutex);
				sleep(1);//wait cancel signal
				pthread_testcancel();
			}
		}

		this->thread_pool->leader_id = this->id;
		pthread_mutex_unlock(this->thread_pool->pool_mutex);

		//get event from msg_queue
		//handle the event
		this->handler(this, this->event_handler);

		//reenter monitor to serialize the program
		pthread_mutex_lock(this->thread_pool->pool_mutex);
		//increase spare threads count
		this->thread_pool->spare_treads_count++;
	}

	pthread_cleanup_pop(0);
	return NULL;
}

static void init_thread(thread_pool_t* thread_pool, int id, event_handler_t* event_handler)
{
	thread_t* thread = &thread_pool->threads[id];
	thread->id = id;
	thread->handler = handle_event;
	thread->event_handler = event_handler;
	thread->thread_pool = thread_pool;
	thread->canceled = 0;

	//be careful about this function
	pthread_create(&(thread->pthread), NULL, thread_do, thread);
}

/*====================================THREAD POOL====================================*/

static pthread_cond_t* alloc_thread_cond(int threads_count)
{
	int i, j, err;

	pthread_cond_t* pthread_cond_arr;
	pthread_cond_arr = (pthread_cond_t* )zmalloc(sizeof(pthread_cond_t)* threads_count);
	if(pthread_cond_arr == NULL)
	{
		err_ret("error when allocate thread conditions");
		return NULL;
	}

	for(i = 0; i < threads_count; i++)
	{
		err = pthread_cond_init(&(pthread_cond_arr[i]), NULL);
		if(err != 0)
		{
			for(j = 0; j < i; j++)
				pthread_cond_destroy(&(pthread_cond_arr[j]));
			zfree(pthread_cond_arr);
			err_ret("error when allocate thread conditions");
			return NULL;
		}
	}
	return pthread_cond_arr;
}

static void destroy_thread_cond(thread_pool_t* this)
{
	int i;
	int threads_count = this->threads_count;
	pthread_cond_t* pool_condition = this->pool_condition;

	for(i = 0; i < threads_count; i++)
	{
		pthread_cond_destroy(&pool_condition[i]);
	}
	zfree(this->pool_condition);
}

thread_pool_t* alloc_thread_pool(int threads_count, void* msg_queue, 
		resolve_handler_t resolve_handler)
{
	thread_pool_t *thread_pool;
	thread_pool = (thread_pool_t*)zmalloc(sizeof(thread_pool_t));
	if(thread_pool == NULL)
	{
		err_ret("error in allocate thread_pool");
		return NULL;
	}

	thread_pool->threads = (thread_t* )zmalloc(sizeof(thread_t) * threads_count);
	thread_pool->spare_stack = (thread_t* )zmalloc(sizeof(thread_t) * threads_count);
	if(thread_pool->threads == NULL)
	{
		zfree(thread_pool);
		err_ret("error in allocate thread_pool");
		return NULL;
	}
	if(thread_pool->spare_stack == NULL)
	{
		zfree(thread_pool->threads);
		zfree(thread_pool);
		err_ret("error in allocate thread_pool");
		return NULL;
	}

	thread_pool->tp_ops = (threadpool_op_t* )zmalloc(sizeof(threadpool_op_t));
	if(thread_pool->tp_ops == NULL)
	{
		zfree(thread_pool->spare_stack);
		zfree(thread_pool->threads);
		zfree(thread_pool);
		err_ret("error in allocate thread_pool");
		return NULL;
	}

	thread_pool->pool_mutex = (pthread_mutex_t* )zmalloc(sizeof(pthread_mutex_t));
	if(pthread_mutex_init(thread_pool->pool_mutex, NULL) != 0)
	{
		if(thread_pool->pool_mutex)
			zfree(thread_pool->pool_mutex);
		thread_pool->pool_mutex = NULL;
	}
	if(thread_pool->pool_mutex == NULL)
	{
		zfree(thread_pool->tp_ops);
		zfree(thread_pool->spare_stack);
		zfree(thread_pool->threads);
		zfree(thread_pool);
		err_ret("error in allocate thread_pool");
		return NULL;
	}
	thread_pool->handle_mutex = (pthread_mutex_t* )zmalloc(sizeof(pthread_mutex_t));
	if(pthread_mutex_init(thread_pool->handle_mutex, NULL) != 0)
	{
		if(thread_pool->handle_mutex)
			zfree(thread_pool->handle_mutex);
		thread_pool->handle_mutex = NULL;
	}
	if(thread_pool->handle_mutex == NULL)
	{
		zfree(thread_pool->pool_mutex);
		zfree(thread_pool->tp_ops);
		zfree(thread_pool->spare_stack);
		zfree(thread_pool->threads);
		zfree(thread_pool);
		err_ret("error in allocate thread_pool");
		return NULL;
	}

	thread_pool->pool_condition = alloc_thread_cond(threads_count);
	if(thread_pool->pool_condition == NULL)
	{
		pthread_mutex_destroy(thread_pool->handle_mutex);
		zfree(thread_pool->handle_mutex);
		pthread_mutex_destroy(thread_pool->pool_mutex);
		zfree(thread_pool->pool_mutex);
		zfree(thread_pool->tp_ops);
		zfree(thread_pool->spare_stack);
		zfree(thread_pool->threads);
		zfree(thread_pool);
		err_ret("error in allocate thread_pool");
		return NULL;
	}

	thread_pool->msg_queue = msg_queue;
	thread_pool->spare_stack_top = 0;
	thread_pool->spare_treads_count = threads_count;
	thread_pool->threads_count = threads_count;
	thread_pool->pool_status = THREAD_POOL_UNINIT;
	thread_pool->leader_id = NO_THREAD_LEADER;

	if((thread_pool->handler_set = 
			alloc_event_handler_set(thread_pool, resolve_handler)) == NULL)
	{
		destroy_thread_cond(thread_pool);
		pthread_mutex_destroy(thread_pool->handle_mutex);
		zfree(thread_pool->handle_mutex);
		pthread_mutex_destroy(thread_pool->pool_mutex);
		zfree(thread_pool->pool_mutex);
		zfree(thread_pool->tp_ops);
		zfree(thread_pool->spare_stack);
		zfree(thread_pool->threads);
		zfree(thread_pool);
		err_ret("error in allocate thread_pool");
		return NULL;
	}

	//initial thread pool operations
	thread_pool->tp_ops->promote_a_leader = promote_a_leader;
	thread_pool->tp_ops->deactive_handle = deactive_handle;
	thread_pool->tp_ops->reactive_handle = reactive_handle;
	thread_pool->tp_ops->start = start;
	return thread_pool;
}

static int push_stack(thread_pool_t* this, thread_t* thread)
{
	thread_t* spare_stack = this->spare_stack;
	int top = this->spare_stack_top;
	if(top == this->threads_count)
		return -1;
	spare_stack[top] = *thread;
	this->spare_stack_top = this->spare_stack_top + 1;
#ifdef THREAD_POOL_DEBUG
	log_write(LOG_DEBUG, "push thread %d into spare stack", thread->id);
#endif
	return 0;
}

static int pop_stack(thread_pool_t* this, thread_t* stack_top)
{
	thread_t* spare_stack = this->spare_stack;
	int top = this->spare_stack_top;
	if(top == 0)
	{
#ifdef THREAD_POOL_DEBUG
		log_write(LOG_DEBUG, "now the spare stack is empty");
#endif
		return -1;
	}
	*stack_top = spare_stack[top - 1];
	this->spare_stack_top = this->spare_stack_top - 1;
#ifdef THREAD_POOL_DEBUG
	log_write(LOG_DEBUG, "pop thread %d from spare stack", stack_top->id);
#endif
	return 0;
}

//the function will lock the message queue
static void deactive_handle(thread_pool_t* this)
{
	if(pthread_mutex_lock(this->handle_mutex) != 0)
		err_ret("wrong in deactive handle function");
}

//this function will unlock the message queue
static void reactive_handle(thread_pool_t* this)
{
	if(pthread_mutex_unlock(this->handle_mutex) != 0)
		err_ret("wrong in reactive handle function");
}

/**
 * promote a new leader to handler the new message
 */
static int promote_a_leader(thread_pool_t* this, thread_t* thread)
{
	int pop_status;
	thread_t new_leader;
	pthread_mutex_lock(this->pool_mutex);
	if(this->leader_id != thread->id)
	{
		pthread_mutex_unlock(this->pool_mutex);
		return -1;
	}
	pop_status = pop_stack(this, &new_leader);
	//if there are spare thread in the stack, promote it else set leader id to NO_THREAD_LEADER
	if(pop_status != -1)
	{
		this->leader_id = new_leader.id;
		pthread_mutex_unlock(this->pool_mutex);

		//awake a thread
		pthread_cond_signal(this->pool_condition + new_leader.id);
	}
	else
	{
		this->leader_id = NO_THREAD_LEADER;
		pthread_mutex_unlock(this->pool_mutex);
	}
	return new_leader.id;
}

static void start(thread_pool_t* this)
{
	int i;
	event_handler_t** handler_set = this->handler_set->event_handler_arr;
#ifdef THREAD_POOL_DEBUG
	log_write(LOG_DEBUG, "Now in start function and will start thread pool soon!");
#endif
	if(this->pool_status == THREAD_POOL_UNINIT)
	{
		//the first thread will be the leader
		pthread_mutex_lock(this->pool_mutex);
		this->leader_id = 0;
		pthread_mutex_unlock(this->pool_mutex);

		for(i = 0; i < this->threads_count; i++)
		{
			init_thread(this, i, handler_set[i]);
			//push_stack(this, &this->threads[i]);
		}
		this->pool_status = THREAD_POOL_INIT;
	}
	return;
}

/**
 * destroy thread pool
 * this function may be not right, should be careful
 */
void destroy_thread_pool(thread_pool_t* thread_pool)
{
	int i, stack_num;
	int threads_count = thread_pool->threads_count;
	thread_t* threads = thread_pool->threads;
	thread_t thread_temp;

	sleep(1);//wait until threads are initial

	//wait until threads finish its' job
	while(thread_pool->spare_treads_count < thread_pool->threads_count)
		sleep(1);

#ifdef THREAD_POOL_DEBUG
	log_write(LOG_DEBUG, "The number of spare threads is %d", thread_pool->spare_treads_count);
#endif

	//cancel threads
	for(i = 0; i < threads_count; i++)
	{
		thread_pool->threads[i].canceled = 1;
	}

	stack_num = thread_pool->spare_stack_top;
	for(i = 0; i < stack_num; i++)
	{
		pop_stack(thread_pool, &thread_temp);
#ifdef THREAD_POOL_DEBUG
		log_write(LOG_DEBUG, "thread %d is going to be killed", thread_temp.id);
#endif
		pthread_cond_signal(thread_pool->pool_condition + thread_temp.id);
	}

	//awake thread who is locked by handler
	thread_pool->tp_ops->reactive_handle(thread_pool);

	//send cancel signal
	for(i = 0; i < threads_count; i++)
		pthread_cancel(threads[i].pthread);
	for(i = 0; i < threads_count; i++)
		pthread_join(threads[i].pthread, NULL);

	destroy_thread_cond(thread_pool);
	pthread_mutex_destroy(thread_pool->handle_mutex);
	zfree(thread_pool->handle_mutex);
	pthread_mutex_destroy(thread_pool->pool_mutex);
	zfree(thread_pool->pool_mutex);
	zfree(thread_pool->tp_ops);
	zfree(thread_pool->spare_stack);
	zfree(thread_pool->threads);
	//free event handler set
	destroy_event_handler_set(thread_pool->handler_set);
	zfree(thread_pool);
}
