#include "../../../global.h"
#include "../basic_structure.h"
#include <stdio.h>

int main()
{
	char * mem_file_system;
	int groups_count, blocks_per_group;
	int i, j;
	super_block_t *mem_super_block;
	group_desc_block_t *mem_group_block;

	printf("%d\n", sizeof(super_block));
	//format file system
	mem_file_system = init_mem_file_system(LARGE, 0);
	mem_super_block = (super_block_t *)mem_file_system;
	groups_count = mem_super_block->s_blocks_count / mem_super_block->s_blocks_per_group;
	blocks_per_group = mem_super_block->s_blocks_per_group;

	//check all group blocks
	for(i = 0; i < groups_count; i++)
	{
		mem_super_block = (super_block_t *)mem_file_system;
		mem_group_block = (group_desc_block_t *)(mem_file_system + BLOCK_SIZE);

		for(j = 0; j < groups_count; j++)
		{
			mem_group_block = mem_group_block + 1;
		}

		printf("the bit map first data is %lu\n", *(unsigned long *)(mem_file_system + BLOCK_SIZE * 2));

		mem_file_system = mem_file_system + BLOCK_SIZE * blocks_per_group;
	}

	return 0;
}
