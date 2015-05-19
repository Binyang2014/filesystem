#include <stdio.h>
#include "../bitmap.h"
int main()
{
	unsigned long bitmap[2];
	int i, nbits = 64;
	printf("the ans is %lu\n", ffs(0x04));
	bitmap_zero(bitmap, nbits * 2);
	bitmap_set(bitmap, 0, 1);
	bitmap_set(bitmap, 73, 1);
	printf("%lu\n", bitmap[0]);
	for(i = 0; i < 1; i++ )
	{
		printf("%x\t", bitmap[i]);
		printf("%x\t", &bitmap[i]);
		printf("%x\t", *((int*)bitmap + 1));
		printf("%x\t", ((int*)bitmap + 1));
	}
	printf("%d\t", bitmap_a_bit_full(bitmap, 0));
	printf("%d\t", bitmap_a_bit_empty(bitmap, 0));
	printf("%d\t", bitmap_a_bit_full(bitmap, 74));
	printf("%d\t", bitmap_a_bit_empty(bitmap, 74));

	printf("\n");
	printf("bitmap weight is %d\n", bitmap_weight(bitmap, nbits));
	return 0;
}
