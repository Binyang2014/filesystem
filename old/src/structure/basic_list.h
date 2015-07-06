/**
 * created on 2015.5.11
 * author: Binyang
 * This is a basic two-way list file and many data structure will use this
 * list to implement.
 * This list is like Redis' adlist
 *
 * list is used to implement buffer
 */
#ifndef _BASIC_LIST_H_
#define _BASIC_LIST_H_

/* weather allocate a new room for a node */
#define NO_ALLOC 0
#define DO_ALLOC 1
/* Directions for iterators */
#define AL_START_HEAD 0
#define AL_START_TAIL 1

/* Functions implemented as macros */
#define list_length(l) ((l)->len)
#define list_first(l) ((l)->head)
#define list_last(l) ((l)->tail)
#define list_prev_node(n) ((n)->prev)
#define list_next_node(n) ((n)->next)
#define list_node_value(n) ((n)->value)

#define list_set_node_value(n,v) ((n)->value = (v))

#define list_set_dup_method(l,m) ((l)->dup = (m))
#define list_set_free_method(l,m) ((l)->free = (m))
#define list_set_match_method(l,m) ((l)->match = (m))

#define list_get_dup_method(l) ((l)->dup)
#define list_get_free(l) ((l)->free)
#define list_get_match_method(l) ((l)->match)

struct node;
struct list_iter;
struct list_operations;
struct list;

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

struct list_operations
{
	struct list* (*list_add_node_head)(struct list *list, void *value);
	struct list* (*list_add_exist_node_head)(struct list *list, struct node *node);
	struct list* (*list_add_node_tail)(struct list *list, void *value);
	struct list* (*list_add_exist_node_tail)(struct list *list, struct node *node);
	struct list* (*list_insert_node)(struct list *list, struct node *old_node,
			void *value, int after);
	void (*list_del_node)(struct list *list, struct node *node);
	struct list_iter* (*list_get_iterator)(struct list *list, int direction);
	struct node* (*list_next)(struct list_iter *iter);
	void (*list_release_iterator)(struct list_iter *iter);
	struct list* (*list_dup)(struct list *orig);
	struct node* (*list_search_key)(struct list *list, void *key);
	struct node* (*list_index)(struct list *list, long index);
	void (*list_rewind)(struct list *list, struct list_iter *li);
	void (*list_rewind_tail)(struct list *list, struct list_iter *li);
	void (*list_rotate)(struct list *list);
};

struct list
{
    struct node *head;
    struct node *tail;
    struct list_operations* list_ops;
    void *(*dup)(void *ptr);
    void (*free)(void *ptr);
    int (*match)(void *ptr, void *key);
    unsigned long len;
};

/* Typedef */
typedef struct list list_t;
typedef struct node list_node_t;
typedef struct list_iter list_iter_t;
typedef struct list_operations list_op_t;

/* Prototypes */
list_t *list_create();
void list_release(list_t *list);
void list_release_without_node(list_t *list);

#endif
