#include <stdio.h>
#include "bitmap.h"
int main()
{
	unsigned long bitmap[1 << 6];
	int i, nbits = 1<<12;
	bitmap_full(bitmap, nbits);
	for(i = 0; i < (1 << 6); i++ )
		printf("%lu\t", bitmap[i]);
	printf("\n");
	printf("bitmap weight is %d\n", bitmap_weight(bitmap, nbits));
	return 0;
}
