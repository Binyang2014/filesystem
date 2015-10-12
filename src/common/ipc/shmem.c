/* created on: 2015.10.12
 * author: binyang
 *
 * impelement functions in shmem.h
 */
#include "shmem.h"
#include "zmalloc.h"

shmem_t *create_shm(uint32_t key, size_t size, int flag)
{
	shmem_t *shmem = zmalloc(sizeof(shmem_t));
	if(shmem == NULL)
		return NULL;
	if(size > MAX_SHM_SIZE)
	{
		zfree(shmem);
		return NULL;
	}
	shmem->key = key;
	shmem->shmid = shmget(key, size, flag | IPC_CREATE);
	shmem->addr = NULL;
	sem = sem_open("/tmp/");

	return shmem;
}
