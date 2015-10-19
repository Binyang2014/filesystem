/* created on: 2015.10.12
 * author: binyang
 *
 * impelement functions in shmem.h
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "shmem.h"
#include "zmalloc.h"

shmem_t *create_shm(uint32_t key, size_t size, int flag)
{
	char sem_path[20];
	struct shmid_ds buf;
	shmem_t *shmem = zmalloc(sizeof(shmem_t));

	if(shmem == NULL)
		return NULL;
	if(size > MAX_SHM_SIZE)
	{
		zfree(shmem);
		return NULL;
	}
	shmem->key = key;
	shmem->shmid = shmget(key, size, flag | IPC_CREAT | IPC_EXCL);
	shmem->addr = NULL;
	shmctl(shmem->shmid, IPC_STAT, &buf);
	shmem->size = buf.shm_segsz;
	sprintf(sem_path, "/e-%d", shmem->shmid);
	shmem->e_sem = sem_open(sem_path, O_CREAT | O_EXCL, S_IRWXU, 1);
	sprintf(sem_path, "/f-%d", shmem->shmid);
	shmem->f_sem = sem_open(sem_path, O_CREAT, S_IRWXU, 0);

	return shmem;
}

shmem_t *get_shm(uint32_t key, int flag)
{
	char sem_path[20];
	shmem_t *shmem = zmalloc(sizeof(shmem_t));
	struct shmid_ds buf;

	if(shmem == NULL)
		return NULL;
	shmem->shmid = shmget(key, 0, flag);
	shmem->key = key;
	shmem->addr = NULL;
	shmctl(shmem->shmid, IPC_STAT, &buf);
	shmem->size = buf.shm_segsz;
	sprintf(sem_path, "/e-%d", shmem->shmid);
	shmem->e_sem = sem_open(sem_path, 0);
	sprintf(sem_path, "/f-%d", shmem->shmid);
	shmem->f_sem = sem_open(sem_path, 0);

	return shmem;
}

void *attach_shm(shmem_t *shmem)
{
	if(shmem->addr != NULL)
		return shmem->addr;
	shmem->addr = shmat(shmem->shmid, 0, 0);
	if(shmem->addr == (void *)-1)
		return NULL;
	return shmem->addr;
}

void detach_shm(shmem_t *shmem)
{
	if(shmem->addr != NULL)
	{
		shmdt(shmem->addr);
		shmem->addr = NULL;
	}
}

void destroy_shm(shmem_t *shmem)
{
	sem_close(shmem->e_sem);
	sem_close(shmem->f_sem);
	shmctl(shmem->shmid, IPC_RMID, 0);
	zfree(shmem);
}

void shm_free(struct shmem *shmem)
{
	sem_close(shmem->e_sem);
	sem_close(shmem->f_sem);
	zfree(shmem);
}

int send_to_shm(shmem_t *shmem, void *data, size_t data_len)
{
	void *addr = shmem->addr;

	if( (data_len + sizeof(shmem_head_t)) > shmem->size )
		return -1;
	sem_wait(shmem->e_sem);
	((shmem_head_t *)addr)->shm_length = data_len;
	addr = addr + sizeof(shmem_head_t);
	memcpy(addr, data, data_len);
	sem_post(shmem->f_sem);
	return 0;
}

int recv_shm_with_len(shmem_t *shmem, void *data, size_t data_len)
{
	void *addr = shmem->addr;
	size_t len;

	if( (data_len + sizeof(shmem_head_t)) > shmem->size )
		return -1;
	sem_wait(shmem->f_sem);
	len = ((shmem_head_t *)addr)->shm_length;
	if(len < data_len)
		data_len = len;
	addr = addr + sizeof(shmem_head_t);
	memcpy(data, addr, data_len);
	sem_post(shmem->e_sem);
	return 0;
}

int recv_from_shm(struct shmem *shmem, void *data)
{
	void *addr = shmem->addr;
	size_t len;

	sem_wait(shmem->f_sem);
	len = ((shmem_head_t *)addr)->shm_length;
	addr = addr + sizeof(shmem_head_t);
	memcpy(data, addr, len);
	sem_post(shmem->e_sem);
	return 0;
}
