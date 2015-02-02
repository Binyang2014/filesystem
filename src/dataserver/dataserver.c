#include "dataserver.h"
#include "type/basic_structure.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
//this is a demo, there are many things to add
int get_current_imformation(struct data_server * server_imf)
{
	FILE* fp;
	char temp[100];
	char *parameter;
	unsigned int total_mem, free_mem;
	fp = fopen("/proc/meminfo", "r");
	if(fp == NULL)
	{
		perror("can not get memory information");
		return 1;
	}

	fgets(temp, 100, fp);
	strtok(temp, " ");//分割字符串
	parameter = strtok(NULL, " ");
	server_imf->memory_used = strtoul(parameter, NULL, 10);

	fgets(temp, 100, fp);
	strtok(temp, " ");
	parameter = strtok(NULL, " ");
	server_imf->memory_free = strtoul(parameter, NULL, 10);
	server_imf->memory_used -= server_imf->memory_free;

	fclose(fp);
	return 0;
}

void init_dataserver()
{
	char* total_blocks = (char *)malloc(sizeof(char) * MAX_ALLOC_SIZE);
	if(total_blocks == NULL)
	{
		perror("can not alloc such big memory");
		return;
	}
	printf("alloc successfully\n");
}
