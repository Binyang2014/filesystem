/*
 * bitmap.h
 * create on 2015.1.28
 * author:Binyang
 *
 * Thanks to Linux contributors who wrote this program
 */
#ifndef _BITMAP_H_
#define _BITMAP_H_

#include <string.h>
#include <linux/types.h>

static const int BITS_PER_LONG = sizeof(long) * 8;
#define BITS_TO_LONG(nbits) ( (nbits + BITS_PER_LONG - 1) / BITS_PER_LONG)//除法的时候向上取整
#define BIT_WORD(nr) ( (nr) / BITS_PER_LONG)
#define BITMAP_FIRST_WORD_MASK(start) (~0UL << ((start) % BITS_PER_LONG))
#define BITMAP_LAST_WORD_MASK(nbits)	                \
(                                                       \
	((nbits) % BITS_PER_LONG) ?                         \
		(1UL<<((nbits) % BITS_PER_LONG))-1 : ~0UL       \
)
#define __ALIGN_MASK(x, mask) (((x) + (mask)) & ~(mask))
/**
* ffz - find first zero in word.
* @word: The word to search
*
* Undefined if no zero exists, so code should check against ~0UL first.
*/
#define ffz(x) __ffs(~(x))


/**
 * find first set bit, this not return 0 when word is 0, the complete
 * function finished in ffs()
 */
static inline unsigned long __ffs(unsigned long word)
{
	int num = 0;
	if (BITS_PER_LONG == 64)
	{
		if ((word & 0xffffffff) == 0) {
			num += 32;
			word >>= 32;
		 }
	}//高位移动到低位
	if ((word & 0xffff) == 0)
	{
		num += 16;
		word >>= 16;
	}
	if ((word & 0xff) == 0)
	{
		num += 8;
		word >>= 8;
	}
	if ((word & 0xf) == 0)
	{
		num += 4;
		word >>= 4;
	}
	if ((word & 0x3) == 0)
	{
		num += 2;
		word >>= 2;
	}
	if ((word & 0x1) == 0)
		num += 1;
	return num;
}

/**
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
/**
 * set bitmap to 1
 */
static inline void bitmap_fill(unsigned long *dst, int nbits)
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


/**
 * complete in bitmap.c
 */
int bitmap_weight(const unsigned long *bitmap, int nbits);
int bitmap_empty(unsigned long *bitmap, int nbits);
int bitmap_full(const unsigned long *src, unsigned int nbits);
void bitmap_set(unsigned long *bitmap, unsigned int start, int len);
void bitmap_clear(unsigned long *bitmap, unsigned int start, int len);
int ffs(int x);
unsigned long find_first_zero_bit(const unsigned long *addr, unsigned long size);
unsigned long find_next_zero_bit(const unsigned long *addr, unsigned long size,
                                     unsigned long offset);
unsigned long bitmap_find_next_zero_area(unsigned long *map,
                                              unsigned long size,
                                              unsigned long start,
                                              unsigned int nr,
                                              unsigned long align_mask);
unsigned long find_first_bit(const unsigned long *addr, unsigned long size);
unsigned long find_next_bit(const unsigned long *addr, unsigned long size,
                               unsigned long offset);
#endif
