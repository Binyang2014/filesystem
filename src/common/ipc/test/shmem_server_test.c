#include <unistd.h>
#include "shmem.h"
#include "log.h"

int main()
{
	shmem_t *shmem;
	char data[] = "This is a shared memory";

	log_init("", LOG_DEBUG);
	shmem = create_shm(COMMON_KEY, MAX_SHM_SIZE, UREAD | UWRITE);
	attach_shm(shmem);
	send_to_shm(shmem, data, sizeof(data));
	sleep(10);
	detach_shm(shmem);
	destroy_shm(shmem);
	return 0;
}
