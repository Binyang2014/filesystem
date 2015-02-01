#include "basic_structure.h"
#include <stdio.h>
int main()
{
	char * mem_file_system;
	int groups_count, i;
	super_block_t *mem_super_block;
	mem_file_system = init_mem_file_system(LARGE, 0);
	mem_super_block = (super_block_t *)mem_file_system;
	groups_count = mem_super_block->s_blocks_count / mem_super_block->s_blocks_per_group;
	for(i = 0; i < groups_count; i++)
	{
		mem_super_block = (super_block_t *)mem_file_system;
		mem_file_system = mem_file_system + BLOCK_SIZE * mem_super_block->s_blocks_per_group;
	}
	return 0;
}
