/*
 * hash.h
 *
 * created on: 2015.3.8
 * author: Binyang
 *
 * this file define some hash functions
 */
#ifndef _HASH_H_
#define _HASH_H_
#define MAX_NUM_SIZE 20

#include <stdlib.h>

unsigned int simple_hash(char* str, unsigned int size);
unsigned int PJWHash(char* str, unsigned int size);
unsigned int bkdr_hash(const char *str, int length);
unsigned int bddr_long_hash(unsigned long value, int size);
//将unsigned long long 转化成字符数组
char* ulltoa(unsigned long long num, char* str);

#endif
