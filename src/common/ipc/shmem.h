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

#define SHM_UREAD 0400
#define SHM_UWRITE 0200
#define SHM_UEXCUTE 0100
#define SHM_GREAD 0040
#define SHM_GWRITE 0020
#define SHM_GEXCUTE 0010
#define SHM_OREAD 0004
#define SHM_OWRITE 0002
#define SHM_OEXCUTE 0001

#define COMMON_KEY 8317

struct shmem_head
{
	size_t shm_length;
};

#define MAX_SHM_SIZE (MAX_DATA_CONTENT_LEN + sizeof(struct shmem_head))

struct shmem
{
	int shmid;
	uint32_t key;
	void *addr;
	sem_t *f_sem;
	sem_t *e_sem;
	size_t size;
};

struct shmem *create_shm(uint32_t key, size_t size, int flag);
struct shmem *get_shm(uint32_t key, int flag);
void *attach_shm(struct shmem *shmem);
void detach_shm(struct shmem *shmem);
void destroy_shm(struct shmem *shmem);
void shm_free(struct shmem *shmem);

int send_to_shm(struct shmem *shmem, void *data, size_t data_len);
int recv_shm_with_len(struct shmem *shmem, void *data, size_t data_len);
int recv_from_shm(struct shmem *shmem, void *data);
void wait_esem(struct shmem *shmem);
void post_esem(struct shmem *shmem);
void wait_fsem(struct shmem *shmem);
void post_fsem(struct shmem *shmem);

typedef struct shmem shmem_t;
typedef struct shmem_head shmem_head_t;

#endif
