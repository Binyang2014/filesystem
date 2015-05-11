/**
 * created on 2015.5.11
 * author: Binyang
 * This is a basic two-way list file and many data structure will use this
 * list to implement.
 * This list is like Redis' adlist
 *
 * list is used to implement buffer
 */
#ifndef _BAISIC_LIST_H_
#define _BASIC_LIST_H_

/* Directions for iterators */
#define AL_START_HEAD 0
#define AL_START_TAIL 1

/* Functions implemented as macros */
#define list_length(l) ((l)->len)
#define list_first(l) ((l)->head)
#define list_last(l) ((l)->tail)
#define list_prevNode(n) ((n)->prev)
#define list_nextNode(n) ((n)->next)
#define list_node_value(n) ((n)->value)

#define list_set_dup_method(l,m) ((l)->dup = (m))
#define list_set_free_method(l,m) ((l)->free = (m))
#define list_set_match_method(l,m) ((l)->match = (m))

#define list_get_dup_method(l) ((l)->dup)
#define list_get_free(l) ((l)->free)
#define list_get_match_method(l) ((l)->match)

struct node
{
	void* value;
	struct node* next;
	struct node* prev;
};

struct list_iter
{
    struct node *next;
    int direction;
};

struct list
{
    struct node *head;
    struct node *tail;
    void *(*dup)(void *ptr);
    void (*free)(void *ptr);
    int (*match)(void *ptr, void *key);
    unsigned long len;
};

/* Typedef */
typedef struct list list_t;
typedef struct node list_node_t;
typedef struct list_iter list_iter_t;

/* Prototypes */
list_t *list_create(void);
void list_release(list_t *list);
list_t *list_add_node_head(list_t *list, void *value);
list_t *list_add_node_tail(list_t *list, void *value);
list_t *list_insert_node(list_t *list, list_node_t *old_node, void *value, int after);
void list_del_node(list_t *list, list_node_t *node);
list_iter_t *list_get_iterator(list_t *list, int direction);
list_node_t *list_next(list_iter_t *iter);
void list_release_iterator(list_iter_t *iter);
list_t *list_dup(list_t *orig);
list_node_t *listS_search_key(list_t *list, void *key);
list_node_t *list_index(list_t *list, long index);
void list_rewind(list_t *list, list_iter_t *li);
void list_rewind_tail(list_t *list, list_iter_t *li);
void list_rotate(list_t *list);

//struct buffer
//{
//	void *buffer;
//	struct buffer* next;
//};
//
//typedef struct buffer buffer_t;

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
