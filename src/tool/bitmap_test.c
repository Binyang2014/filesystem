#include <stdio.h>
#include "bitmap.h"
int main()
{
	unsigned long bitmap[1];
	int i, nbits = 64;
	printf("the ans is %lu\n", ffs(0x04));
//	bitmap_zero(bitmap, nbits);
//	bitmap_set(bitmap, 0, 1);
//	printf("%lu\n", bitmap[0]);
//	for(i = 0; i < 1; i++ )
//	{
//		printf("%x\t", bitmap[i]);
//		printf("%x\t", &bitmap[i]);
//		printf("%x\t", *((int*)bitmap + 1));
//		printf("%x\t", ((int*)bitmap + 1));
//	}
//	printf("\n");
//	printf("bitmap weight is %d\n", bitmap_weight(bitmap, nbits));
	return 0;
}
