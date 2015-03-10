#include <stdio.h>
#include "hash.h"

int main()
{
	unsigned long int test = ~0;
	char str[MAX_NUM_SIZE + 1];
	ultoa(test, str);
	printf("%s\n", str);
	return 0;
}
