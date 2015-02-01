/*
 * to finish rest function in bitmap.h
 * author:Binyang
 * created:2015.1.28
 * reference bitmap.c in Linux
 */
#include "bitmap.h"
#include <stdio.h>

//internal functions @Linux
static int __bitmap_empty(const unsigned long *bitmap, unsigned int bits)
{
	unsigned int k, lim = bits/BITS_PER_LONG;
	for (k = 0; k < lim; ++k)
		if (bitmap[k])
			return 0;

	if (bits % BITS_PER_LONG)
		if (bitmap[k] & BITMAP_LAST_WORD_MASK(bits))
			return 0;

	return 1;
}

static int __bitmap_full(const unsigned long *bitmap, unsigned int bits)
{
	unsigned int k, lim = bits/BITS_PER_LONG;
	for (k = 0; k < lim; ++k)
		if (~bitmap[k])
			return 0;

	if (bits % BITS_PER_LONG)
		if (~bitmap[k] & BITMAP_LAST_WORD_MASK(bits))
			return 0;

	return 1;
}

/**
 * bitmap_find_next_zero_area_off - find a contiguous aligned zero area
 * @map: The address to base the search on
 * @size: The bitmap size in bits
 * @start: The bitnumber to start searching at
 * @nr: The number of zeroed bits we're looking for
 * @align_mask: Alignment mask for zero area
 * @align_offset: Alignment offset for zero area.
 *
 * The @align_mask should be one less than a power of 2; the effect is that
 * the bit offset of all zero areas this function finds plus @align_offset
 * is multiple of that power of 2.
 */
//static unsigned long bitmap_find_next_zero_area_off(unsigned long *map,
//					     unsigned long size,
//					     unsigned long start,
//					     unsigned int nr,
//					     unsigned long align_mask,
//					     unsigned long align_offset)
//{
//	unsigned long index, end, i;
//again:
//	index = find_next_zero_bit(map, size, start);
//
//	/* Align allocation */
//	index = __ALIGN_MASK(index + align_offset, align_mask) - align_offset;
//
//	end = index + nr;
//	if (end > size)
//		return end;
//	i = find_next_bit(map, end, index);
//	if (i < end) {
//		start = i + 1;
//		goto again;
//	}
//	return index;
//}
//implement

/**
 * calculate numbers of 1 in a long type
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

////next functions you can find in linux/lib/bitmap.c
void bitmap_set(unsigned long *bitmap, unsigned int start, int len)
{
	unsigned long *p = bitmap + BIT_WORD(start);
	const unsigned int size = start + len;
	int bits_to_set = BITS_PER_LONG - (start % BITS_PER_LONG);
	unsigned long mask_to_set = BITMAP_FIRST_WORD_MASK(start);
    while (len - bits_to_set >= 0)
    {
    	*p |= mask_to_set;
    	len -= bits_to_set;
    	bits_to_set = BITS_PER_LONG;
    	mask_to_set = ~0UL;
    	p++;
    }
    if (len)
    {
    	mask_to_set &= BITMAP_LAST_WORD_MASK(size);
    	*p |= mask_to_set;
    }
}

void bitmap_clear(unsigned long *bitmap, unsigned int start, int len)
{
	unsigned long *p = bitmap + BIT_WORD(start);
	const unsigned int size = start + len;
	int bits_to_clear = BITS_PER_LONG - (start % BITS_PER_LONG);
	unsigned long mask_to_clear = BITMAP_FIRST_WORD_MASK(start);

	while (len - bits_to_clear >= 0) {
		*p &= ~mask_to_clear;
		len -= bits_to_clear;
		bits_to_clear = BITS_PER_LONG;
		mask_to_clear = ~0UL;
		p++;
	}
	if (len) {
		mask_to_clear &= BITMAP_LAST_WORD_MASK(size);
		*p &= ~mask_to_clear;
	}
}

int bitmap_empty(unsigned long *bitmap, int nbits)
{
	if ( nbits < BITS_PER_LONG )
		return ! (*bitmap & BITMAP_LAST_WORD_MASK(nbits));
	else
		return __bitmap_empty(bitmap, nbits);
}

int bitmap_full(const unsigned long *src, unsigned int nbits)
{
	if (nbits < BITS_PER_LONG)
		return ! (~(*src) & BITMAP_LAST_WORD_MASK(nbits));
	else
		return __bitmap_full(src, nbits);
}

/**
* ffs - find first bit set
* @x: the word to search
*
* This is defined the same way as
* the libc and compiler builtin ffs routines, therefore
* differs in spirit from the above ffz (man ffs).
* Note ffs(0) = 0, ffs(1) = 1, ffs(0x80000000) = 32.
*/
int ffs(int x)
{
	if (!x)
		return 0;

	return __ffs(x) + 1;
}

//
//unsigned long bitmap_find_next_zero_area(unsigned long *map,
//			   	   	   	   	   	   	   	   	  unsigned long size,
//											  unsigned long start,
//											  unsigned int nr,
//											  unsigned long align_mask)
//{
//	return bitmap_find_next_zero_area_off(map, size, start, nr,
//						      align_mask, 0);
//}
//
unsigned long find_first_zero_bit(const unsigned long *addr, unsigned long size)
{
	const unsigned long *p = addr;
	unsigned long result = 0;
	unsigned long tmp;

	while (size & ~(BITS_PER_LONG-1)) {//相当于除法，由于BITS_PER_LONG是2的整数倍
		if (~(tmp = *(p++)))
			goto found;
		result += BITS_PER_LONG;
		size -= BITS_PER_LONG;
	}
	if (!size)
		return result;

	tmp = (*p) | (~0UL << size);
	if (tmp == ~0UL)        /* Are any bits zero? */
		return result + size;   /* Nope. */
found:
	return result + ffz(tmp);
}
