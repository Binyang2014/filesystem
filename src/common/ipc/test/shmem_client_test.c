#include "shmem.h"
#include "log.h"

int main()
{
	shmem_t *shmem;
	char data[50];

	log_init("", LOG_DEBUG);
	shmem = get_shm(COMMON_KEY, UREAD | UWRITE);
	attach_shm(shmem);
	recv_from_shm(shmem, data);
	detach_shm(shmem);
	destroy_shm(shmem);
	printf("data is %s\n", data);

	return 0;
}
