#include <stdio.h>
#include "../basic_list.h"

void list_test()
{
	list_t* list;
	int len;
	list = list_create();
	len = list_length(list);
	printf("the lenght of the list is %d\n", len);
}

int main()
{
	list_test();
	return 0;
}
