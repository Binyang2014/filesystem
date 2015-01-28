#include <stdio.h>
#include "dataserver.h"
#include <pthread.h>
int main()
{
	data_server first;
	int ret;
	pthread_mutex_t m;
	printf("%d\n", sizeof(m));
	init_dataserver();
	ret = get_current_imformation(&first);
	if(ret == 1)
	{
		perror("something wrong");
	}
	printf("memory_used is %dMB, free is %dMB\n", first.memory_used>>10, first.memory_free>>10);
	return 0;
}
