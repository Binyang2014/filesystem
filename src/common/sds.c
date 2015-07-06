/* SDSLib, A C dynamic strings library
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "sds.h"
#include "zmalloc.h"

/* Create a new sds string with the content specified by the 'init' pointer
 * and 'initlen'.
 * If NULL is used for 'init' the string is initialized with zero bytes.
 *
 * The string is always null-termined (all the sds strings are, always) so
 * even if you create an sds string with:
 *
 * mystring = sds_new_len("abc",3);
 *
 * You can print the string with printf() as there is an implicit \0 at the
 * end of the string. However the string is binary safe and can contain
 * \0 characters in the middle, as the length is stored in the sds header. */
sds sds_new_len(const void *init, size_t init_len) {
    struct sdshdr *sh;

    if (init) {
        sh = zmalloc(sizeof(sdshdr) + init_len + 1);
    } else {
        sh = zcalloc(sizeof(struct sdshdr) + init_len + 1);
    }
    if (sh == NULL) return NULL;
    sh->len = init_len;
    sh->free = 0;
    if (init_len && init){
        memcpy(sh->buf, init, init_len);
    }
    sh->buf[init_len] = '\0';
    return (char*)sh->buf;
}

/* Create an empty (zero length) sds string. Even in this case the string
 * always has an implicit null term. */
sds sds_empty(void) {
    return sds_new_len("", 0);
}

/* Create a new sds string starting from a null termined C string. */
sds sds_new(const char *init) {
    size_t init_len = (init == NULL) ? 0 : strlen(init);
    return sds_new_len(init, init_len);
}

/* Duplicate an sds string. */
sds sds_dup(const sds s) {
    return sds_new_len(s, sds_len(s));
}

/* Free an sds string. No operation is performed if 's' is NULL. */
void sds_free(sds s) {
    if (s == NULL) return;
    zfree(s-sizeof(struct sdshdr));
}

/* Set the sds string length to the length as obtained with strlen(), so
 * considering as content only up to the first null term character.
 *
 * This function is useful when the sds string is hacked manually in some
 * way, like in the following example:
 *
 * s = sds_new("foobar");
 * s[2] = '\0';
 * sds_update_len(s);
 * printf("%d\n", sds_len(s));
 *
 * The output will be "2", but if we comment out the call to sds_update_len()
 * the output will be "6" as the string was modified but the logical length
 * remains 6 bytes. */
void sds_update_len(sds s) {
    struct sdshdr *sh = (void*) (s-(sizeof(struct sdshdr)));
    int reallen = strlen(s);
    sh->free += (sh->len-reallen);
    sh->len = reallen;
}

/* Modify an sds string in-place to make it empty (zero length).
 * However all the existing buffer is not discarded but set as free space
 * so that next append operations will not require allocations up to the
 * number of bytes previously available. */
void sds_clear(sds s) {
    struct sdshdr *sh = (void*) (s-(sizeof(struct sdshdr)));
    sh->free += sh->len;
    sh->len = 0;
    sh->buf[0] = '\0';
}

/* Enlarge the free space at the end of the sds string so that the caller
 * is sure that after calling this function can overwrite up to addlen
 * bytes after the end of the string, plus one more byte for nul term.
 *
 * Note: this does not change the *length* of the sds string as returned
 * by sds_len(), but only the free buffer space we have. */
sds sds_make_room_for(sds s, size_t addlen) {
    struct sdshdr *sh, *newsh;
    size_t free = sds_avail(s);
    size_t len, newlen;

    if (free >= addlen) return s;
    len = sds_len(s);
    sh = (void*) (s-(sizeof(struct sdshdr)));
    newlen = (len + addlen);
    if (newlen < SDS_MAX_PREALLOC)
        newlen *= 2;
    else
        newlen += SDS_MAX_PREALLOC;
    newsh = zrealloc(sh, sizeof(struct sdshdr)+newlen+1);
    if (newsh == NULL) return NULL;

    newsh->free = newlen - len;
    return newsh->buf;
}

/* Reallocate the sds string so that it has no free space at the end. The
 * contained string remains not altered, but next concatenation operations
 * will require a reallocation.
 *
 * After the call, the passed sds string is no longer valid and all the
 * references must be substituted with the new pointer returned by the call. */
sds sds_remove_free_space(sds s) {
    struct sdshdr *sh;

    sh = (void*) (s-(sizeof(struct sdshdr)));
    sh = zrealloc(sh, sizeof(struct sdshdr)+sh->len+1);
    sh->free = 0;
    return sh->buf;
}

/* Return the total size of the allocation of the specifed sds string,
 * including:
 * 1) The sds header before the pointer.
 * 2) The string.
 * 3) The free buffer at the end if any.
 * 4) The implicit null term.
 */
size_t sds_alloc_size(sds s) {
    struct sdshdr *sh = (void*) (s-(sizeof(struct sdshdr)));

    return sizeof(*sh)+sh->len+sh->free+1;
}

/* Increment the sds length and decrements the left free space at the
 * end of the string according to 'incr'. Also set the null term
 * in the new end of the string.
 *
 * This function is used in order to fix the string length after the
 * user calls sds_make_room_for(), writes something after the end of
 * the current string, and finally needs to set the new length.
 *
 * Note: it is possible to use a negative increment in order to
 * right-trim the string.
 *
 * Usage example:
 *
 * Using sds_incr_len() and sds_make_room_for() it is possible to mount the
 * following schema, to cat bytes coming from the kernel to the end of an
 * sds string without copying into an intermediate buffer:
 *
 * oldlen = sds_len(s);
 * s = sds_make_room_for(s, BUFFER_SIZE);
 * nread = read(fd, s+oldlen, BUFFER_SIZE);
 * ... check for nread <= 0 and handle it ...
 * sds_incr_len(s, nread);
 */
void sds_incr_len(sds s, int incr) {
    struct sdshdr *sh = (void*) (s-(sizeof(struct sdshdr)));

    if (incr >= 0)
        assert(sh->free >= (unsigned int)incr);
    else
        assert(sh->len >= (unsigned int)(-incr));
    sh->len += incr;
    sh->free -= incr;
    s[sh->len] = '\0';
}

/* Grow the sds to have the specified length. Bytes that were not part of
 * the original length of the sds will be set to zero.
 *
 * if the specified length is smaller than the current length, no operation
 * is performed. */
sds sds_grow_zero(sds s, size_t len) {
    struct sdshdr *sh = (void*)(s-(sizeof(struct sdshdr)));
    size_t totlen, curlen = sh->len;

    if (len <= curlen) return s;
    s = sds_make_room_for(s,len-curlen);
    if (s == NULL) return NULL;

    /* Make sure added region doesn't contain garbage */
    sh = (void*)(s-(sizeof(struct sdshdr)));
    memset(s+curlen,0,(len-curlen+1)); /* also set trailing \0 byte */
    totlen = sh->len+sh->free;
    sh->len = len;
    sh->free = totlen-sh->len;
    return s;
}

/* Append the specified binary-safe string pointed by 't' of 'len' bytes to the
 * end of the specified sds string 's'.
 *
 * After the call, the passed sds string is no longer valid and all the
 * references must be substituted with the new pointer returned by the call. */
sds sds_cat_len(sds s, const void *t, size_t len) {
    struct sdshdr *sh;
    size_t curlen = sds_len(s);

    s = sds_make_room_for(s,len);
    if (s == NULL) return NULL;
    sh = (void*) (s-(sizeof(struct sdshdr)));
    memcpy(s+curlen, t, len);
    sh->len = curlen+len;
    sh->free = sh->free-len;
    s[curlen+len] = '\0';
    return s;
}

/* Append the specified null termianted C string to the sds string 's'.
 *
 * After the call, the passed sds string is no longer valid and all the
 * references must be substituted with the new pointer returned by the call. */
sds sds_cat(sds s, const char *t) {
    return sds_cat_len(s, t, strlen(t));
}

/* Append the specified sds 't' to the existing sds 's'.
 *
 * After the call, the modified sds string is no longer valid and all the
 * references must be substituted with the new pointer returned by the call. */
sds sds_cat_sds(sds s, const sds t) {
    return sds_cat_len(s, t, sds_len(t));
}

/* Destructively modify the sds string 's' to hold the specified binary
 * safe string pointed by 't' of length 'len' bytes. */
sds sds_cpy_len(sds s, const char *t, size_t len) {
    struct sdshdr *sh = (void*) (s-(sizeof(struct sdshdr)));
    size_t totlen = sh->free+sh->len;

    if (totlen < len) {
        s = sds_make_room_for(s,len-sh->len);
        if (s == NULL) return NULL;
        sh = (void*) (s-(sizeof(struct sdshdr)));
        totlen = sh->free+sh->len;
    }
    memcpy(s, t, len);
    s[len] = '\0';
    sh->len = len;
    sh->free = totlen-len;
    return s;
}

/* Like sds_cpy_len() but 't' must be a null-termined string so that the length
 * of the string is obtained with strlen(). */
sds sds_cpy(sds s, const char *t) {
    return sds_cpy_len(s, t, strlen(t));
}

/* Helper for sds_catlonglong() doing the actual number -> string
 * conversion. 's' must point to a string with room for at least
 * SDS_LLSTR_SIZE bytes.
 *
 * The function returns the length of the null-terminated string
 * representation stored at 's'. */
#define SDS_LLSTR_SIZE 21
int sdsll2str(char *s, long long value) {
    char *p, aux;
    unsigned long long v;
    size_t l;

    /* Generate the string representation, this method produces
     * an reversed string. */
    v = (value < 0) ? -value : value;
    p = s;
    do {
        *p++ = '0'+(v%10);
        v /= 10;
    } while(v);
    if (value < 0) *p++ = '-';

    /* Compute length and add null term. */
    l = p-s;
    *p = '\0';

    /* Reverse the string. */
    p--;
    while(s < p) {
        aux = *s;
        *s = *p;
        *p = aux;
        s++;
        p--;
    }
    return l;
}

/* Identical sdsll2str(), but for unsigned long long type. */
int sdsull2str(char *s, unsigned long long v) {
    char *p, aux;
    size_t l;

    /* Generate the string representation, this method produces
     * an reversed string. */
    p = s;
    do {
        *p++ = '0'+(v%10);
        v /= 10;
    } while(v);

    /* Compute length and add null term. */
    l = p-s;
    *p = '\0';

    /* Reverse the string. */
    p--;
    while(s < p) {
        aux = *s;
        *s = *p;
        *p = aux;
        s++;
        p--;
    }
    return l;
}

/* Create an sds string from a long long value. It is much faster than:
 *
 * sds_cat_printf(sds_empty(),"%lld\n", value);
 */
sds sds_from_long_long(long long value) {
    char buf[SDS_LLSTR_SIZE];
    int len = sdsll2str(buf,value);

    return sds_new_len(buf,len);
}

/* Like sds_cat_printf() but gets va_list instead of being variadic. */
sds sds_cat_vprintf(sds s, const char *fmt, va_list ap) {
    va_list cpy;
    char staticbuf[1024], *buf = staticbuf, *t;
    size_t buflen = strlen(fmt)*2;

    /* We try to start using a static buffer for speed.
     * If not possible we revert to heap allocation. */
    if (buflen > sizeof(staticbuf)) {
        buf = zmalloc(buflen);
        if (buf == NULL) return NULL;
    } else {
        buflen = sizeof(staticbuf);
    }

    /* Try with buffers two times bigger every time we fail to
     * fit the string in the current buffer size. */
    while(1) {
        buf[buflen-2] = '\0';
        va_copy(cpy,ap);
        vsnprintf(buf, buflen, fmt, cpy);
        va_end(cpy);
        if (buf[buflen-2] != '\0') {
            if (buf != staticbuf) zfree(buf);
            buflen *= 2;
            buf = zmalloc(buflen);
            if (buf == NULL) return NULL;
            continue;
        }
        break;
    }

    /* Finally concat the obtained string to the SDS string and return it. */
    t = sds_cat(s, buf);
    if (buf != staticbuf) zfree(buf);
    return t;
}

/* Append to the sds string 's' a string obtained using printf-alike format
 * specifier.
 *
 * After the call, the modified sds string is no longer valid and all the
 * references must be substituted with the new pointer returned by the call.
 *
 * Example:
 *
 * s = sds_new("Sum is: ");
 * s = sds_cat_printf(s,"%d+%d = %d",a,b,a+b).
 *
 * Often you need to create a string from scratch with the printf-alike
 * format. When this is the need, just use sds_empty() as the target string:
 *
 * s = sds_cat_printf(sds_empty(), "... your format ...", args);
 */
sds sds_cat_printf(sds s, const char *fmt, ...) {
    va_list ap;
    char *t;
    va_start(ap, fmt);
    t = sds_cat_vprintf(s,fmt,ap);
    va_end(ap);
    return t;
}

/* This function is similar to sds_cat_printf, but much faster as it does
 * not rely on sprintf() family functions implemented by the libc that
 * are often very slow. Moreover directly handling the sds string as
 * new data is concatenated provides a performance improvement.
 *
 * However this function only handles an incompatible subset of printf-alike
 * format specifiers:
 *
 * %s - C String
 * %S - SDS string
 * %i - signed int
 * %I - 64 bit signed integer (long long, int64_t)
 * %u - unsigned int
 * %U - 64 bit unsigned integer (unsigned long long, uint64_t)
 * %% - Verbatim "%" character.
 */
sds sds_cat_fmt(sds s, char const *fmt, ...) {
    struct sdshdr *sh = (void*) (s-(sizeof(struct sdshdr)));
    size_t initlen = sds_len(s);
    const char *f = fmt;
    int i;
    va_list ap;

    va_start(ap,fmt);
    f = fmt;    /* Next format specifier byte to process. */
    i = initlen; /* Position of the next byte to write to dest str. */
    while(*f) {
        char next, *str;
        unsigned int l;
        long long num;
        unsigned long long unum;

        /* Make sure there is always space for at least 1 char. */
        if (sh->free == 0) {
            s = sds_make_room_for(s,1);
            sh = (void*) (s-(sizeof(struct sdshdr)));
        }

        switch(*f) {
        case '%':
            next = *(f+1);
            f++;
            switch(next) {
            case 's':
            case 'S':
                str = va_arg(ap,char*);
                l = (next == 's') ? strlen(str) : sds_len(str);
                if (sh->free < l) {
                    s = sds_make_room_for(s,l);
                    sh = (void*) (s-(sizeof(struct sdshdr)));
                }
                memcpy(s+i,str,l);
                sh->len += l;
                sh->free -= l;
                i += l;
                break;
            case 'i':
            case 'I':
                if (next == 'i')
                    num = va_arg(ap,int);
                else
                    num = va_arg(ap,long long);
                {
                    char buf[SDS_LLSTR_SIZE];
                    l = sdsll2str(buf,num);
                    if (sh->free < l) {
                        s = sds_make_room_for(s,l);
                        sh = (void*) (s-(sizeof(struct sdshdr)));
                    }
                    memcpy(s+i,buf,l);
                    sh->len += l;
                    sh->free -= l;
                    i += l;
                }
                break;
            case 'u':
            case 'U':
                if (next == 'u')
                    unum = va_arg(ap,unsigned int);
                else
                    unum = va_arg(ap,unsigned long long);
                {
                    char buf[SDS_LLSTR_SIZE];
                    l = sdsull2str(buf,unum);
                    if (sh->free < l) {
                        s = sds_make_room_for(s,l);
                        sh = (void*) (s-(sizeof(struct sdshdr)));
                    }
                    memcpy(s+i,buf,l);
                    sh->len += l;
                    sh->free -= l;
                    i += l;
                }
                break;
            default: /* Handle %% and generally %<unknown>. */
                s[i++] = next;
                sh->len += 1;
                sh->free -= 1;
                break;
            }
            break;
        default:
            s[i++] = *f;
            sh->len += 1;
            sh->free -= 1;
            break;
        }
        f++;
    }
    va_end(ap);

    /* Add null-term */
    s[i] = '\0';
    return s;
}

/* Remove the part of the string from left and from right composed just of
 * contiguous characters found in 'cset', that is a null terminted C string.
 *
 * After the call, the modified sds string is no longer valid and all the
 * references must be substituted with the new pointer returned by the call.
 *
 * Example:
 *
 * s = sds_new("AA...AA.a.aa.aHelloWorld     :::");
 * s = sds_trim(s,"A. :");
 * printf("%s\n", s);
 *
 * Output will be just "Hello World".
 */
sds sds_trim(sds s, const char *cset) {
    struct sdshdr *sh = (void*) (s-(sizeof(struct sdshdr)));
    char *start, *end, *sp, *ep;
    size_t len;

    sp = start = s;
    ep = end = s+sds_len(s)-1;
    while(sp <= end && strchr(cset, *sp)) sp++;
    while(ep > start && strchr(cset, *ep)) ep--;
    len = (sp > ep) ? 0 : ((ep-sp)+1);
    if (sh->buf != sp) memmove(sh->buf, sp, len);
    sh->buf[len] = '\0';
    sh->free = sh->free+(sh->len-len);
    sh->len = len;
    return s;
}

/* Turn the string into a smaller (or equal) string containing only the
 * substring specified by the 'start' and 'end' indexes.
 *
 * start and end can be negative, where -1 means the last character of the
 * string, -2 the penultimate character, and so forth.
 *
 * The interval is inclusive, so the start and end characters will be part
 * of the resulting string.
 *
 * The string is modified in-place.
 *
 * Example:
 *
 * s = sds_new("Hello World");
 * sds_range(s,1,-1); => "ello World"
 */
void sds_range(sds s, int start, int end) {
    struct sdshdr *sh = (void*) (s-(sizeof(struct sdshdr)));
    size_t newlen, len = sds_len(s);

    if (len == 0) return;
    if (start < 0) {
        start = len+start;
        if (start < 0) start = 0;
    }
    if (end < 0) {
        end = len+end;
        if (end < 0) end = 0;
    }
    newlen = (start > end) ? 0 : (end-start)+1;
    if (newlen != 0) {
        if (start >= (signed)len) {
            newlen = 0;
        } else if (end >= (signed)len) {
            end = len-1;
            newlen = (start > end) ? 0 : (end-start)+1;
        }
    } else {
        start = 0;
    }
    if (start && newlen) memmove(sh->buf, sh->buf+start, newlen);
    sh->buf[newlen] = 0;
    sh->free = sh->free+(sh->len-newlen);
    sh->len = newlen;
}

/* Apply tolower() to every character of the sds string 's'. */
void sds_to_lower(sds s) {
    int len = sds_len(s), j;

    for (j = 0; j < len; j++) s[j] = tolower(s[j]);
}

/* Apply toupper() to every character of the sds string 's'. */
void sds_toupper(sds s) {
    int len = sds_len(s), j;

    for (j = 0; j < len; j++) s[j] = toupper(s[j]);
}

/* Compare two sds strings s1 and s2 with memcmp().
 *
 * Return value:
 *
 *     positive if s1 > s2.
 *     negative if s1 < s2.
 *     0 if s1 and s2 are exactly the same binary string.
 *
 * If two strings share exactly the same prefix, but one of the two has
 * additional characters, the longer string is considered to be greater than
 * the smaller one. */
int sds_cmp(const sds s1, const sds s2) {
    size_t l1, l2, minlen;
    int cmp;

    l1 = sds_len(s1);
    l2 = sds_len(s2);
    minlen = (l1 < l2) ? l1 : l2;
    cmp = memcmp(s1,s2,minlen);
    if (cmp == 0) return l1-l2;
    return cmp;
}

/* Split 's' with separator in 'sep'. An array
 * of sds strings is returned. *count will be set
 * by reference to the number of tokens returned.
 *
 * On out of memory, zero length string, zero length
 * separator, NULL is returned.
 *
 * Note that 'sep' is able to split a string using
 * a multi-character separator. For example
 * sdssplit("foo_-_bar","_-_"); will return two
 * elements "foo" and "bar".
 *
 * This version of the function is binary-safe but
 * requires length arguments. sdssplit() is just the
 * same function but for zero-terminated strings.
 */
sds *sds_split_len(const char *s, int len, const char *sep, int seplen, int *count) {
    int elements = 0, slots = 5, start = 0, j;
    sds *tokens;

    if (seplen < 1 || len < 0) return NULL;

    tokens = zmalloc(sizeof(sds)*slots);
    if (tokens == NULL) return NULL;

    if (len == 0) {
        *count = 0;
        return tokens;
    }
    for (j = 0; j < (len-(seplen-1)); j++) {
        /* make sure there is room for the next element and the final one */
        if (slots < elements+2) {
            sds *newtokens;

            slots *= 2;
            newtokens = zrealloc(tokens,sizeof(sds)*slots);
            if (newtokens == NULL) goto cleanup;
            tokens = newtokens;
        }
        /* search the separator */
        if ((seplen == 1 && *(s+j) == sep[0]) || (memcmp(s+j,sep,seplen) == 0)) {
            tokens[elements] = sds_new_len(s+start,j-start);
            if (tokens[elements] == NULL) goto cleanup;
            elements++;
            start = j+seplen;
            j = j+seplen-1; /* skip the separator */
        }
    }
    /* Add the final element. We are sure there is room in the tokens array. */
    tokens[elements] = sds_new_len(s+start,len-start);
    if (tokens[elements] == NULL) goto cleanup;
    elements++;
    *count = elements;
    return tokens;

cleanup:
    {
        int i;
        for (i = 0; i < elements; i++) sds_free(tokens[i]);
        zfree(tokens);
        *count = 0;
        return NULL;
    }
}

/* Free the result returned by sds_split_len(), or do nothing if 'tokens' is NULL. */
void sds_free_split_res(sds *tokens, int count) {
    if (!tokens) return;
    while(count--)
        sds_free(tokens[count]);
    zfree(tokens);
}

/* Append to the sds string "s" an escaped string representation where
 * all the non-printable characters (tested with isprint()) are turned into
 * escapes in the form "\n\r\a...." or "\x<hex-number>".
 *
 * After the call, the modified sds string is no longer valid and all the
 * references must be substituted with the new pointer returned by the call. */
sds sds_cat_repr(sds s, const char *p, size_t len) {
    s = sds_cat_len(s,"\"",1);
    while(len--) {
        switch(*p) {
        case '\\':
        case '"':
            s = sds_cat_printf(s,"\\%c",*p);
            break;
        case '\n': s = sds_cat_len(s,"\\n",2); break;
        case '\r': s = sds_cat_len(s,"\\r",2); break;
        case '\t': s = sds_cat_len(s,"\\t",2); break;
        case '\a': s = sds_cat_len(s,"\\a",2); break;
        case '\b': s = sds_cat_len(s,"\\b",2); break;
        default:
            if (isprint(*p))
                s = sds_cat_printf(s,"%c",*p);
            else
                s = sds_cat_printf(s,"\\x%02x",(unsigned char)*p);
            break;
        }
        p++;
    }
    return sds_cat_len(s,"\"",1);
}

/* Helper function for sds_split_args() that returns non zero if 'c'
 * is a valid hex digit. */
int is_hex_digit(char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
           (c >= 'A' && c <= 'F');
}

/* Helper function for sds_split_args() that converts a hex digit into an
 * integer from 0 to 15 */
int hex_digit_to_int(char c) {
    switch(c) {
    case '0': return 0;
    case '1': return 1;
    case '2': return 2;
    case '3': return 3;
    case '4': return 4;
    case '5': return 5;
    case '6': return 6;
    case '7': return 7;
    case '8': return 8;
    case '9': return 9;
    case 'a': case 'A': return 10;
    case 'b': case 'B': return 11;
    case 'c': case 'C': return 12;
    case 'd': case 'D': return 13;
    case 'e': case 'E': return 14;
    case 'f': case 'F': return 15;
    default: return 0;
    }
}

/* Split a line into arguments, where every argument can be in the
 * following programming-language REPL-alike form:
 *
 * foo bar "newline are supported\n" and "\xff\x00otherstuff"
 *
 * The number of arguments is stored into *argc, and an array
 * of sds is returned.
 *
 * The caller should free the resulting array of sds strings with
 * sds_free_split_res().
 *
 * Note that sds_cat_repr() is able to convert back a string into
 * a quoted string in the same format sds_split_args() is able to parse.
 *
 * The function returns the allocated tokens on success, even when the
 * input string is empty, or NULL if the input contains unbalanced
 * quotes or closed quotes followed by non space characters
 * as in: "foo"bar or "foo'
 */
sds *sds_split_args(const char *line, int *argc) {
    const char *p = line;
    char *current = NULL;
    char **vector = NULL;

    *argc = 0;
    while(1) {
        /* skip blanks */
        while(*p && isspace(*p)) p++;
        if (*p) {
            /* get a token */
            int inq=0;  /* set to 1 if we are in "quotes" */
            int insq=0; /* set to 1 if we are in 'single quotes' */
            int done=0;

            if (current == NULL) current = sds_empty();
            while(!done) {
                if (inq) {
                    if (*p == '\\' && *(p+1) == 'x' &&
                                             is_hex_digit(*(p+2)) &&
                                             is_hex_digit(*(p+3)))
                    {
                        unsigned char byte;

                        byte = (hex_digit_to_int(*(p+2))*16)+
                                hex_digit_to_int(*(p+3));
                        current = sds_cat_len(current,(char*)&byte,1);
                        p += 3;
                    } else if (*p == '\\' && *(p+1)) {
                        char c;

                        p++;
                        switch(*p) {
                        case 'n': c = '\n'; break;
                        case 'r': c = '\r'; break;
                        case 't': c = '\t'; break;
                        case 'b': c = '\b'; break;
                        case 'a': c = '\a'; break;
                        default: c = *p; break;
                        }
                        current = sds_cat_len(current,&c,1);
                    } else if (*p == '"') {
                        /* closing quote must be followed by a space or
                         * nothing at all. */
                        if (*(p+1) && !isspace(*(p+1))) goto err;
                        done=1;
                    } else if (!*p) {
                        /* unterminated quotes */
                        goto err;
                    } else {
                        current = sds_cat_len(current,p,1);
                    }
                } else if (insq) {
                    if (*p == '\\' && *(p+1) == '\'') {
                        p++;
                        current = sds_cat_len(current,"'",1);
                    } else if (*p == '\'') {
                        /* closing quote must be followed by a space or
                         * nothing at all. */
                        if (*(p+1) && !isspace(*(p+1))) goto err;
                        done=1;
                    } else if (!*p) {
                        /* unterminated quotes */
                        goto err;
                    } else {
                        current = sds_cat_len(current,p,1);
                    }
                } else {
                    switch(*p) {
                    case ' ':
                    case '\n':
                    case '\r':
                    case '\t':
                    case '\0':
                        done=1;
                        break;
                    case '"':
                        inq=1;
                        break;
                    case '\'':
                        insq=1;
                        break;
                    default:
                        current = sds_cat_len(current,p,1);
                        break;
                    }
                }
                if (*p) p++;
            }
            /* add the token to the vector */
            vector = zrealloc(vector,((*argc)+1)*sizeof(char*));
            vector[*argc] = current;
            (*argc)++;
            current = NULL;
        } else {
            /* Even on empty input string return something not NULL. */
            if (vector == NULL) vector = zmalloc(sizeof(void*));
            return vector;
        }
    }

err:
    while((*argc)--)
        sds_free(vector[*argc]);
    zfree(vector);
    if (current) sds_free(current);
    *argc = 0;
    return NULL;
}

/* Modify the string substituting all the occurrences of the set of
 * characters specified in the 'from' string to the corresponding character
 * in the 'to' array.
 *
 * For instance: sds_map_chars(mystring, "ho", "01", 2)
 * will have the effect of turning the string "hello" into "0ell1".
 *
 * The function returns the sds string pointer, that is always the same
 * as the input pointer since no resize is needed. */
sds sds_map_chars(sds s, const char *from, const char *to, size_t setlen) {
    size_t j, i, l = sds_len(s);

    for (j = 0; j < l; j++) {
        for (i = 0; i < setlen; i++) {
            if (s[j] == from[i]) {
                s[j] = to[i];
                break;
            }
        }
    }
    return s;
}

/* Join an array of C strings using the specified separator (also a C string).
 * Returns the result as an sds string. */
sds sds_join(char **argv, int argc, char *sep) {
    sds join = sds_empty();
    int j;

    for (j = 0; j < argc; j++) {
        join = sds_cat(join, argv[j]);
        if (j != argc-1) join = sds_cat(join,sep);
    }
    return join;
}

#ifdef SDS_TEST_MAIN
#include <stdio.h>
#include "testhelp.h"
#include "limits.h"

int main(void) {
    {
        struct sdshdr *sh;
        sds x = sds_new("foo"), y;

        test_cond("Create a string and obtain the length",
            sds_len(x) == 3 && memcmp(x,"foo\0",4) == 0)

        sds_free(x);
        x = sds_new_len("foo",2);
        test_cond("Create a string with specified length",
            sds_len(x) == 2 && memcmp(x,"fo\0",3) == 0)

        x = sds_cat(x,"bar");
        test_cond("Strings concatenation",
            sds_len(x) == 5 && memcmp(x,"fobar\0",6) == 0);

        x = sds_cpy(x,"a");
        test_cond("sds_cpy() against an originally longer string",
            sds_len(x) == 1 && memcmp(x,"a\0",2) == 0)

        x = sds_cpy(x,"xyzxxxxxxxxxxyyyyyyyyyykkkkkkkkkk");
        test_cond("sds_cpy() against an originally shorter string",
            sds_len(x) == 33 &&
            memcmp(x,"xyzxxxxxxxxxxyyyyyyyyyykkkkkkkkkk\0",33) == 0)

        sds_free(x);
        x = sds_cat_printf(sds_empty(),"%d",123);
        test_cond("sds_cat_printf() seems working in the base case",
            sds_len(x) == 3 && memcmp(x,"123\0",4) == 0)

        sds_free(x);
        x = sds_new("--");
        x = sds_cat_fmt(x, "Hello %s World %I,%I--", "Hi!", LLONG_MIN,LLONG_MAX);
        test_cond("sds_cat_fmt() seems working in the base case",
            sds_len(x) == 60 &&
            memcmp(x,"--Hello Hi! World -9223372036854775808,"
                     "9223372036854775807--",60) == 0)

        sds_free(x);
        x = sds_new("--");
        x = sds_cat_fmt(x, "%u,%U--", UINT_MAX, ULLONG_MAX);
        test_cond("sds_cat_fmt() seems working with unsigned numbers",
            sds_len(x) == 35 &&
            memcmp(x,"--4294967295,18446744073709551615--",35) == 0)

        sds_free(x);
        x = sds_new("xxciaoyyy");
        sds_trim(x,"xy");
        test_cond("sds_trim() correctly trims characters",
            sds_len(x) == 4 && memcmp(x,"ciao\0",5) == 0)

        y = sds_dup(x);
        sds_range(y,1,1);
        test_cond("sds_range(...,1,1)",
            sds_len(y) == 1 && memcmp(y,"i\0",2) == 0)

        sds_free(y);
        y = sds_dup(x);
        sds_range(y,1,-1);
        test_cond("sds_range(...,1,-1)",
            sds_len(y) == 3 && memcmp(y,"iao\0",4) == 0)

        sds_free(y);
        y = sds_dup(x);
        sds_range(y,-2,-1);
        test_cond("sds_range(...,-2,-1)",
            sds_len(y) == 2 && memcmp(y,"ao\0",3) == 0)

        sds_free(y);
        y = sds_dup(x);
        sds_range(y,2,1);
        test_cond("sds_range(...,2,1)",
            sds_len(y) == 0 && memcmp(y,"\0",1) == 0)

        sds_free(y);
        y = sds_dup(x);
        sds_range(y,1,100);
        test_cond("sds_range(...,1,100)",
            sds_len(y) == 3 && memcmp(y,"iao\0",4) == 0)

        sds_free(y);
        y = sds_dup(x);
        sds_range(y,100,100);
        test_cond("sds_range(...,100,100)",
            sds_len(y) == 0 && memcmp(y,"\0",1) == 0)

        sds_free(y);
        sds_free(x);
        x = sds_new("foo");
        y = sds_new("foa");
        test_cond("sds_cmp(foo,foa)", sds_cmp(x,y) > 0)

        sds_free(y);
        sds_free(x);
        x = sds_new("bar");
        y = sds_new("bar");
        test_cond("sds_cmp(bar,bar)", sds_cmp(x,y) == 0)

        sds_free(y);
        sds_free(x);
        x = sds_new("aar");
        y = sds_new("bar");
        test_cond("sds_cmp(bar,bar)", sds_cmp(x,y) < 0)

        sds_free(y);
        sds_free(x);
        x = sds_new_len("\a\n\0foo\r",7);
        y = sds_cat_repr(sds_empty(),x,sds_len(x));
        test_cond("sds_cat_repr(...data...)",
            memcmp(y,"\"\\a\\n\\x00foo\\r\"",15) == 0)

        {
            int oldfree;

            sds_free(x);
            x = sds_new("0");
            sh = (void*) (x-(sizeof(struct sdshdr)));
            test_cond("sds_new() free/len buffers", sh->len == 1 && sh->free == 0);
            x = sds_make_room_for(x,1);
            sh = (void*) (x-(sizeof(struct sdshdr)));
            test_cond("sds_make_room_for()", sh->len == 1 && sh->free > 0);
            oldfree = sh->free;
            x[1] = '1';
            sds_incr_len(x,1);
            test_cond("sds_incr_len() -- content", x[0] == '0' && x[1] == '1');
            test_cond("sds_incr_len() -- len", sh->len == 2);
            test_cond("sds_incr_len() -- free", sh->free == oldfree-1);
        }
    }
    test_report()
    return 0;
}
#endif
