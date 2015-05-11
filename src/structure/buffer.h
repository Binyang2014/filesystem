/**
 * created on 2015.5.11
 * author: Binyang
 * I don't know where to put this structure, so this is a temporary file
 * and many functions will be redefined later
 */
#ifndef _BUFFER_H_
#define _BUFFER_H_
struct buffer
{
	void *buffer;
	struct buffer* next;
};

typedef struct buffer buffer_t;

/*--------------functions------------
 * alloc_buffer
 * resize_buffer
 * delete_buffer
 * separate buffer
 * get buffer size
 *
 * maybe need buffer list, I will consider it later
 */

#endif
