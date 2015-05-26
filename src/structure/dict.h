/* Hash Tables Implementation.
 *
 * This file implements in-memory hash tables with insert/del/replace/find/
 * get-random-element operations. Hash tables will auto-resize if needed
 * tables of power of two in size are used, collisions are handled by
 * chaining. See the source code for more information... :)
 *
 * Copyright (c) 2006-2012, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdint.h>

#ifndef __DICT_H
#define __DICT_H

#define DICT_OK 0
#define DICT_ERR 1

/* Unused arguments generate annoying warnings... */
#define DICT_NOTUSED(V) ((void) V)

typedef struct dict_entry {
    void *key;
    union {
        void *val;
        uint64_t u64;
        int64_t s64;
        double d;
    } v;
    struct dict_entry *next;
} dict_entry;

typedef struct dict_type {
    unsigned int (*hash_function)(const void *key);
    void *(*key_dup)(void *privdata, const void *key);
    void *(*val_dup)(void *privdata, const void *obj);
    int (*key_compare)(void *privdata, const void *key1, const void *key2);
    void (*key_destructor)(void *privdata, void *key);
    void (*val_destructor)(void *privdata, void *obj);
} dict_type;

/* This is our hash table structure. Every dictionary has two of this as we
 * implement incremental rehashing, for the old to the new table. */
typedef struct dictht {
    dict_entry **table;
    unsigned long size;
    unsigned long sizemask;
    unsigned long used;
} dictht;

typedef struct dict {
    dict_type *type;
    void *privdata;
    dictht ht[2];
    long rehashidx; /* rehashing not in progress if rehashidx == -1 */
    int iterators; /* number of iterators currently running */
} dict;

/* If safe is set to 1 this is a safe iterator, that means, you can call
 * dictAdd, dictFind, and other functions against the dictionary even while
 * iterating. Otherwise it is a non safe iterator, and only dictNext()
 * should be called while iterating. */
typedef struct dict_iterator {
    dict *d;
    long index;
    int table, safe;
    dict_entry *entry, *next_entry;
    /* unsafe iterator fingerprint for misuse detection. */
    long long fingerprint;
} dict_iterator;

typedef void (dict_scan_function)(void *privdata, const dict_entry *de);

/* This is the initial size of every hash table */
#define DICT_HT_INITIAL_SIZE     4

/* ------------------------------- Macros ------------------------------------*/
#define dict_free_val(d, entry) \
    if ((d)->type->val_destructor) \
        (d)->type->val_destructor((d)->privdata, (entry)->v.val)

#define dict_set_val(d, entry, _val_) do { \
    if ((d)->type->val_dup) \
        entry->v.val = (d)->type->val_dup((d)->privdata, _val_); \
    else \
        entry->v.val = (_val_); \
} while(0)

#define dict_set_signed_integer_val(entry, _val_) \
    do { entry->v.s64 = _val_; } while(0)

#define dict_set_unsigned_integer_val(entry, _val_) \
    do { entry->v.u64 = _val_; } while(0)

#define dict_set_double_val(entry, _val_) \
    do { entry->v.d = _val_; } while(0)

#define dict_free_key(d, entry) \
    if ((d)->type->key_destructor) \
        (d)->type->key_destructor((d)->privdata, (entry)->key)

#define dict_set_key(d, entry, _key_) do { \
    if ((d)->type->key_dup) \
        entry->key = (d)->type->key_dup((d)->privdata, _key_); \
    else \
        entry->key = (_key_); \
} while(0)

#define dict_compare_keys(d, key1, key2) \
    (((d)->type->key_compare) ? \
        (d)->type->key_compare((d)->privdata, key1, key2) : \
        (key1) == (key2))

#define dict_hash_key(d, key) (d)->type->hash_function(key)
#define dict_get_key(he) ((he)->key)
#define dict_get_val(he) ((he)->v.val)
#define dict_get_signed_integer_val(he) ((he)->v.s64)
#define dict_get_unsigned_integer_val(he) ((he)->v.u64)
#define dict_get_double_val(he) ((he)->v.d)
#define dict_slots(d) ((d)->ht[0].size+(d)->ht[1].size)
#define dict_size(d) ((d)->ht[0].used+(d)->ht[1].used)
#define dict_is_rehashing(d) ((d)->rehashidx != -1)

/* API */
dict *dict_create(dict_type *type, void *priv_data_ptr);
int dict_expand(dict *d, unsigned long size);
int dict_add(dict *d, void *key, void *val);
dict_entry *dict_add_raw(dict *d, void *key);
int dict_replace(dict *d, void *key, void *val);
dict_entry *dict_replace_raw(dict *d, void *key);
int dict_delete(dict *d, const void *key);
int dict_delete_no_free(dict *d, const void *key);
void dict_release(dict *d);
dict_entry * dict_find(dict *d, const void *key);
void *dict_fetch_value(dict *d, const void *key);
int dict_resize(dict *d);
dict_iterator *dict_get_iterator(dict *d);
dict_iterator *dict_get_safe_iterator(dict *d);
dict_entry *dictNext(dict_iterator *iter);
void dictReleaseIterator(dict_iterator *iter);
dict_entry *dictGetRandomKey(dict *d);
unsigned int dictGetSomeKeys(dict *d, dict_entry **des, unsigned int count);
void dict_print_stats(dict *d);
unsigned int dict_gen_hash_function(const void *key, int len);
unsigned int dict_gen_case_hash_function(const unsigned char *buf, int len);
void dict_empty(dict *d, void(callback)(void*));
void dict_enable_resize(void);
void dict_disable_resize(void);
int dict_rehash(dict *d, int n);
int dict_rehash_milliseconds(dict *d, int ms);
void dict_set_hash_function_seed(unsigned int initval);
unsigned int dict_get_hash_function_seed(void);
unsigned long dict_scan(dict *d, unsigned long v, dict_scan_function *fn, void *privdata);

/* Hash table types */
extern dict_type dict_type_heap_string_copy_key;
extern dict_type dict_type_heap_strings;
extern dict_type dict_type_heap_string_copy_key_value;

#endif /* __DICT_H */
