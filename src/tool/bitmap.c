/*
 * to finish rest function in bitmap.h
 * author:Binyang
 * created:2015.1.28
 */
#include "bitmap.h"
#include <stdio.h>
#include <string.h>
/**
 * caculate numbers of 1 in a long type
 */

int weight_long(unsigned long w)
{
	int weight;
	weight = sizeof(long) == 4 ? weight32(w) : weight64(w);
	return weight;
}

int bitmap_weight(const unsigned long *bitmap, int nbits)
{
	int weight = 0, limit, i;
	limit = nbits/ BITS_PER_LONG;

	for(i = 0; i < limit; i++)
	{
		weight = weight + weight_long(bitmap[i]);
	}

	if(weight % BITS_PER_LONG)
		weight = weight + weight_long(bitmap[limit] & BITMAP_LAST_WORD_MASK(nbits));
	return weight;
}
