/*
 * bitmap.h
 * create on 2015.1.28
 * author:Binyang
 */
#ifndef _BITMAP_H_
#define _BITMAP_H_
#include <string.h>
#include <linux/types.h>

static const BITS_PER_LONG = sizeof(long) * 8;
#define BITS_TO_LONG(nbits) ( (nbits + BITS_PER_LONG - 1) / BITS_PER_LONG)//除法的时候向上取整
#define BITMAP_LAST_WORD_MASK(nbits)					\
(														\
	((nbits) % BITS_PER_LONG) ?							\
		(1UL<<((nbits) % BITS_PER_LONG))-1 : ~0UL		\
)
/*
 *set dst bitmap to zero,we do not sure nbit is valid
 *programmer must confirm it
 */
static inline void bitmap_zero(unsigned long *dst, int nbits)
{
	if(nbits < BITS_PER_LONG)
		*dst = 0UL;
	else
	{
		int len = BITS_TO_LONG(nbits) * sizeof(unsigned long);
		memset(dst, 0, len);
	}
}
/*
 * set bitmap to 1
 */
static inline void bitmap_full(unsigned long *dst, int nbits)
{
	size_t nlongs = BITS_TO_LONG(nbits);
	if(! (nbits < BITS_PER_LONG))
	{
		int len = (nlongs -  1) * sizeof(unsigned long);
		memset(dst, 0xff, len);
	}
	dst[nlongs - 1] = BITMAP_LAST_WORD_MASK(nbits);
}
/**
 * use to calculate weight
 */
static inline int weight32(__u32 w)
{
	int i, weight = 0;
	for(i = 0; i < 32; i++)
	{
		weight = weight + (w & (1 << i) ? 1 : 0);
	}
	return weight;
}
static inline int weight64(__u64 w)
{
	int weight;
	weight = weight32(w) + weight32(w >> 32);
	return weight;
}
int bitmap_weight(const unsigned long *bitmap, int nbits);
int find_first_zero_bit();
void bitmap_set();
void bitmap_clear();
#endif
