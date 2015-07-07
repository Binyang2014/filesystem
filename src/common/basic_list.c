/*
 * This file's copyright blongs to redis
 * modified by: Binyang
 * modified on 2015.5.13
 */

#include <stdlib.h>
#include <assert.h>
#include "basic_list.h"

/*===================== Prototypes ==========================*/
static void init_list_ops(list_op_t* list_ops);
static list_t *list_add_node_head(list_t *list, void *value);
static list_t *list_add_exist_node_head(list_t *list, list_node_t *node);
static list_t *list_extract_node_to_head(list_t *list, list_node_t *node);
static list_t *list_add_node_tail(list_t *list, void *value);
static list_t *list_add_exist_node_tail(list_t *list, list_node_t *node);
static list_t *list_extract_node_to_tail(list_t *list, list_node_t *node);
static list_t *list_insert_node(list_t *list, list_node_t *old_node, void *value, int after);
static void list_del_node(list_t *list, list_node_t *node);
static list_iter_t *list_get_iterator(list_t *list, int direction);
static list_node_t *list_next(list_iter_t *iter);
static void list_release_iterator(list_iter_t *iter);
static list_t *list_dup(list_t *orig);
static list_node_t *list_search_key(list_t *list, void *key);
static list_node_t *list_index(list_t *list, long index);
static void list_rewind(list_t *list, list_iter_t *li);
static void list_rewind_tail(list_t *list, list_iter_t *li);
static void list_rotate(list_t *list);


static void init_list_ops(list_op_t* list_ops)
{
	list_ops->list_add_node_head = list_add_node_head;
	list_ops->list_add_exist_node_tail = list_add_exist_node_head;
	list_ops->list_extract_node_to_head = list_extract_node_to_head;
	list_ops->list_add_node_tail = list_add_node_tail;
	list_ops->list_add_exist_node_tail = list_add_exist_node_tail;
	list_ops->list_extract_node_to_tail = list_extract_node_to_tail;
	list_ops->list_del_node = list_del_node;
	list_ops->list_dup = list_dup;
	list_ops->list_get_iterator = list_get_iterator;
	list_ops->list_index = list_index;
	list_ops->list_insert_node = list_insert_node;
	list_ops->list_next = list_next;
	list_ops->list_release_iterator = list_release_iterator;
	list_ops->list_rewind = list_rewind;
	list_ops->list_rewind_tail = list_rewind_tail;
	list_ops->list_rotate = list_rotate;
	list_ops->list_search_key = list_search_key;
}

/* Create a new list. The created list can be freed with
 * AlFreeList(), but private value of every node need to be freed
 * by the user before to call AlFreeList().
 *
 * On error, NULL is returned. Otherwise the pointer to the new list. */
list_t *list_create()
{
    list_t *list;

    if ((list = (list_t* )malloc(sizeof(list_t))) == NULL)
        return NULL;
    list->head = list->tail = NULL;
    list->len = 0;
    list->dup = NULL;
    list->free = NULL;
    list->match = NULL;

    if((list->list_ops = (list_op_t* )malloc(sizeof(list_op_t))) == NULL)
    {
    	free(list);
    	return NULL;
    }
    init_list_ops(list->list_ops);
    return list;
}

/* Free the whole list.
 *
 * This function can't fail. */
void list_release(list_t *list)
{
    unsigned long len;
    list_node_t *current, *next;

    current = list->head;
    len = list->len;
    while(len--) {
        next = current->next;
        if (list->free) list->free(current->value);//TODO else? free(current->value)
        free(current);
        current = next;
    }
    free(list->list_ops);
    free(list);
}

void list_release_without_node(list_t *list)
{
	unsigned long len;
	list_node_t *current, *next;

    current = list->head;
    len = list->len;
    while(len--) {
        next = current->next;
        if (list->free) list->free(current->value);
        current->prev = NULL;
        current->next = NULL;
        current = next;
    }
    free(list->list_ops);
    free(list);
}

/* Add a new node to the list, to head, containing the specified 'value'
 * pointer as value.
 *
 * On error, NULL is returned and no operation is performed (i.e. the
 * list remains unaltered).
 * On success the 'list' pointer you pass to the function is returned. */
static list_t *list_add_node_head(list_t *list, void *value)
{
    list_node_t *node;

    if ((node = malloc(sizeof(*node))) == NULL)
        return NULL;
    node->value = value;
    if (list->len == 0) {
        list->head = list->tail = node;
        node->prev = node->next = NULL;
    } else {
        node->prev = NULL;
        node->next = list->head;
        list->head->prev = node;
        list->head = node;
    }
    list->len++;
    return list;
}

/*
 * extract a node from the node and add to head
 * TODO check this
 */
struct list* list_extract_node_to_head(struct list *list, struct node *node) {
	if(node == NULL || list->head == node) {
		return list;
	}

	if(node->prev) {
		node->prev->next = node->next;
		node->next = list->head;
	}

	if(node->next) {
		node->next->prev = node->prev;
		node->prev = NULL;
	}

	list->head = node;
	return list;
}

/* Add a new node to the list, to head, not allocate node structure in
 * the list.
 */
static list_t *list_add_exist_node_head(list_t *list, list_node_t *node)
{
    if (list->len == 0) {
        list->head = list->tail = node;
        node->prev = node->next = NULL;
    } else {
        node->prev = NULL;
        node->next = list->head;
        list->head->prev = node;
        list->head = node;
    }
    list->len++;
    return list;
}

/* Add a new node to the list, to tail, containing the specified 'value'
 * pointer as value.
 *
 * On error, NULL is returned and no operation is performed (i.e. the
 * list remains unaltered).
 * On success the 'list' pointer you pass to the function is returned. */
static list_t *list_add_node_tail(list_t *list, void *value)
{
    list_node_t *node;

    if ((node = malloc(sizeof(*node))) == NULL)
        return NULL;
    node->value = value;
    if (list->len == 0) {
        list->head = list->tail = node;
        node->prev = node->next = NULL;
    } else {
        node->prev = list->tail;
        node->next = NULL;
        list->tail->next = node;
        list->tail = node;
    }
    list->len++;
    return list;
}

/* Add a new node to the list, to tail, not allocate node structure in
 * the list.
 */
static list_t *list_add_exist_node_tail(list_t *list, list_node_t *node)
{
    if (list->len == 0) {
        list->head = list->tail = node;
        node->prev = node->next = NULL;
    } else {
        node->prev = list->tail;
        node->next = NULL;
        list->tail->next = node;
        list->tail = node;
    }
    list->len++;
    return list;
}

struct list* list_extract_node_to_tail(struct list *list, struct node *node) {

	if(node == NULL || node == list->tail) {
		return list;
	}

	if(node->prev) {
		node->prev->next = node->next;
		node->next = NULL;
	}

	if(node->next) {
		node->next->prev = node->prev;
		node->prev = list->tail;
	}

	list->tail = node;
	return list;
}

static list_t *list_insert_node(list_t *list, list_node_t *old_node, void *value, int after)
{
    list_node_t *node;

    if ((node = malloc(sizeof(*node))) == NULL)
        return NULL;
    node->value = value;
    if (after) {
        node->prev = old_node;
        node->next = old_node->next;
        if (list->tail == old_node) {
            list->tail = node;
        }
    } else {
        node->next = old_node;
        node->prev = old_node->prev;
        if (list->head == old_node) {
            list->head = node;
        }
    }
    if (node->prev != NULL) {
        node->prev->next = node;
    }
    if (node->next != NULL) {
        node->next->prev = node;
    }
    list->len++;
    return list;
}

/* Remove the specified node from the specified list.
 * It's up to the caller to free the private value of the node.
 *
 * This function can't fail. */
static void list_del_node(list_t *list, list_node_t *node)
{
    if (node->prev)
        node->prev->next = node->next;
    else
        list->head = node->next;
    if (node->next)
        node->next->prev = node->prev;
    else
        list->tail = node->prev;
    if (list->free) list->free(node->value);
    free(node);
    list->len--;
}

/* Returns a list iterator 'iter'. After the initialization every
 * call to listNext() will return the next element of the list.
 *
 * This function can't fail. */
static list_iter_t *list_get_iterator(list_t *list, int direction)
{
	if(list == NULL){
		return NULL;
	}

    list_iter_t *iter;

    if ((iter = malloc(sizeof(list_iter_t))) == NULL) return NULL;
    if (direction == AL_START_HEAD)
        iter->next = list->head;
    else
        iter->next = list->tail;
    iter->direction = direction;
    return iter;
}

/* Release the iterator memory */
static void list_release_iterator(list_iter_t *iter) {
    free(iter);
}

/* Create an iterator in the list private iterator structure */
static void list_rewind(list_t *list, list_iter_t *li) {
    li->next = list->head;
    li->direction = AL_START_HEAD;
}

static void list_rewind_tail(list_t *list, list_iter_t *li) {
    li->next = list->tail;
    li->direction = AL_START_TAIL;
}

/* Return the next element of an iterator.
 * It's valid to remove the currently returned element using
 * listDelNode(), but not to remove other elements.
 *
 * The function returns a pointer to the next element of the list,
 * or NULL if there are no more elements, so the classical usage patter
 * is:
 *
 * iter = listGetIterator(list,<direction>);
 * while ((node = listNext(iter)) != NULL) {
 *     doSomethingWith(list_node_tValue(node));
 * }
 *
 * */
static list_node_t *list_next(list_iter_t *iter)
{
    list_node_t *current = iter->next;

    if (current != NULL) {
        if (iter->direction == AL_START_HEAD)
            iter->next = current->next;
        else
            iter->next = current->prev;
    }
    return current;
}

/* Duplicate the whole list. On out of memory NULL is returned.
 * On success a copy of the original list is returned.
 *
 * The 'Dup' method set with listSetDupMethod() function is used
 * to copy the node value. Otherwise the same pointer value of
 * the original node is used as value of the copied node.
 *
 * The original list both on success or error is never modified. */
static list_t *list_dup(list_t *orig)
{
    list_t *copy;
    list_iter_t *iter;
    list_node_t *node;

    if ((copy = list_create()) == NULL)
        return NULL;
    copy->dup = orig->dup;
    copy->free = orig->free;
    copy->match = orig->match;
    iter = list_get_iterator(orig, AL_START_HEAD);
    while((node = list_next(iter)) != NULL) {
        void *value;

        if (copy->dup) {
            value = copy->dup(node->value);
            if (value == NULL) {
                list_release(copy);
                list_release_iterator(iter);
                return NULL;
            }
        } else
            value = node->value;
        if (list_add_node_tail(copy, value) == NULL) {
            list_release(copy);
            list_release_iterator(iter);
            return NULL;
        }
    }
    list_release_iterator(iter);
    return copy;
}

/* Search the list for a node matching a given key.
 * The match is performed using the 'match' method
 * set with listSetMatchMethod(). If no 'match' method
 * is set, the 'value' pointer of every node is directly
 * compared with the 'key' pointer.
 *
 * On success the first matching node pointer is returned
 * (search starts from head). If no matching node exists
 * NULL is returned. */
static list_node_t *list_search_key(list_t *list, void *key)
{
    list_iter_t *iter;
    list_node_t *node;

    iter = list_get_iterator(list, AL_START_HEAD);
    while((node = list_next(iter)) != NULL) {
        if (list->match) {
            if (list->match(node->value, key)) {
                list_release_iterator(iter);
                return node;
            }
        } else {
            if (key == node->value) {
                list_release_iterator(iter);
                return node;
            }
        }
    }
    list_release_iterator(iter);
    return NULL;
}

/* Return the element at the specified zero-based index
 * where 0 is the head, 1 is the element next to head
 * and so on. Negative integers are used in order to count
 * from the tail, -1 is the last element, -2 the penultimate
 * and so on. If the index is out of range NULL is returned. */
static list_node_t *list_index(list_t *list, long index) {
    list_node_t *n;

    if (index < 0) {
        index = (-index)-1;
        n = list->tail;
        while(index-- && n) n = n->prev;
    } else {
        n = list->head;
        while(index-- && n) n = n->next;
    }
    return n;
}

/* Rotate the list removing the tail node and inserting it to the head. */
static void list_rotate(list_t *list) {
    list_node_t *tail = list->tail;

    if (list_length(list) <= 1) return;

    /* Detach current tail */
    list->tail = tail->prev;
    list->tail->next = NULL;
    /* Move it as head */
    list->head->prev = tail;
    tail->prev = NULL;
    tail->next = list->head;
    list->head = tail;
}
