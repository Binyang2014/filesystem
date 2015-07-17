/* SDSLib, A C dynamic strings library
 *
 * Copyright (c) 2006-2010, Salvatore Sanfilippo <antirez at gmail dot com>
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

#ifndef __SDS_H
#define __SDS_H

#define SDS_MAX_PREALLOC (1024*1024)

#include <sys/types.h>
#include <stdint.h>
#include <stdarg.h>

typedef char *sds;

typedef struct sdshdr {
    unsigned int len;
    unsigned int free;
    char buf[];
}sdshdr;

static inline size_t sds_len(const sds s) {
    sdshdr *sh = (void*)(s-(sizeof(sdshdr)));
    return sh->len;
}

static inline size_t sds_avail(const sds s) {
    sdshdr *sh = (void*)(s-(sizeof(sdshdr)));
    return sh->free;
}

sds sds_new_len(const void *init, size_t init_len);
sds sds_new(const char *init);
sds sds_new_ull(uint64_t num);
sds sds_empty(void);
size_t sds_len(const sds s);
sds sds_dup(const sds s);
void sds_free(sds s);
size_t sds_avail(const sds s);
sds sds_grow_zero(sds s, size_t len);
sds sds_cat_len(sds s, const void *t, size_t len);
sds sds_cat(sds s, const char *t);
sds sds_cat_sds(sds s, const sds t);
sds sds_cpy_len(sds s, const char *t, size_t len);
sds sds_cpy(sds s, const char *t);

sds sds_cat_vprintf(sds s, const char *fmt, va_list ap);
#ifdef __GNUC__
sds sds_cat_printf(sds s, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));
#else
sds sds_cat_printf(sds s, const char *fmt, ...);
#endif

sds sds_cat_fmt(sds s, char const *fmt, ...);
sds sds_trim(sds s, const char *cset);
void sds_range(sds s, int start, int end);
void sds_update_len(sds s);
void sds_clear(sds s);
int sds_cmp(const sds s1, const sds s2);
sds *sds_split_len(const char *s, int len, const char *sep, int seplen, int *count);
void sds_free_split_res(sds *tokens, int count);
void sds_to_lower(sds s);
void sds_to_upper(sds s);
sds sds_from_long_long(long long value);
sds sds_cat_repr(sds s, const char *p, size_t len);
sds *sds_split_args(const char *line, int *argc);
sds sds_map_chars(sds s, const char *from, const char *to, size_t setlen);
sds sds_join(char **argv, int argc, char *sep);

/* Low level functions exposed to the user API */
sds sds_make_room_for(sds s, size_t addlen);
void sds_incr_len(sds s, int incr);
sds sds_remove_free_space(sds s);
size_t sds_alloc_size(sds s);

#endif
