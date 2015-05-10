/*
 * hash.c
 *
 * created on: 2015.3.8
 * author: Binyang
 *
 * this file complete some hash functions
 */

#include "hash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned int simple_hash(char* str, unsigned int size)
{
	int seed = 27;
	size_t origin = strtoul(str, NULL, 0);
	return (origin % seed) % size;
}

unsigned int PJWHash(char* str, unsigned int size)
{
	unsigned int BitsInUnsignedLong = (unsigned int)(4 * 8);
	unsigned int ThreeQuarters     = (unsigned int)((BitsInUnsignedLong  * 3) / 4);
	unsigned int OneEighth         = (unsigned int)(BitsInUnsignedLong / 8);
	unsigned int HighBits          = (unsigned int)(0xFFFFFFFF) << (BitsInUnsignedLong - OneEighth);
	unsigned int hash              = 0;
	unsigned int test              = 0;
	int i;
	for(i = 0; i < strlen(str); i++)
	{
		hash = (hash << OneEighth) + str[i];
		if((test = hash & HighBits)  != 0)
		{
			hash = (( hash ^ (test >> ThreeQuarters)) & (~HighBits));
		}
	}
	return hash % size;
}

char* ulltoa(unsigned long long num, char* str)
{
	int count = 0, i;
	char *temp;
	while(num != 0)
	{
		str[count++] = num % 10;
		num = num / 10;
	}
	str[count] = '\0';

	temp = (char *)malloc(count);
	for(i = 0; count - 1 - i >= 0; i++)
	{
		temp[i] = str[count - 1 - i] + '0';
	}
	temp[count] = '\0';
	strcpy(str, temp);
	return str;
}

unsigned int bkdr_hash(const char *str, int length)
{
	unsigned int seed = 131;// 31 131 1313 13131 131313 etc..
	unsigned int hash = 0;
	while (*str)
	{
		hash = hash*seed + (*str++);
	}
	return(hash % length);
}
