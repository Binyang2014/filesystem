/* shmem.h
 * created on: 2015.10.12
 * author: binyang
 *
 * This file is going to provide some common IPC functions which use XIS share
 * memory mechanism to impelement them.
 */
#ifndef SHMEM_H_
#define SHMEM_H_

#include <stdint.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/shm.h>
#include "global.h"

#define UREAD 0400
#define UWRITE 0200
#define UEXCUTE 0100
#define GREAD 0040
#define GWRITE 0020
#define GEXCUTE 0010
#define OREAD 0004
#define OWRITE 0002
#define OEXCUTE 0001

struct shmem_head
{
	int shm_length;
};

#define MAX_SHM_SIZE (MAX_DATA_CONTENT_LEN + sizeof(struct shmem_head))

struct shmem
{
	int shmid;
	uint32_t key;
	void *addr;
	sem_t *sem;
};

struct shmem *create_shm(uint32_t key, size_t size, int flag);
int get_shm(struct shmem *shmem);
void *attach_shm(struct shmem *shmem, int flag);
void detach_shm(void *addr);
void destroy_shm(struct shmem *shmem);

typedef struct shmem shmem_t;

#endif
