#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/msg.h>
#include "zmalloc.h"
#include "msg_ipc.h"

int create_msq(uint32_t key, mode_t mode)
{
	int msqid;

	msqid = msgget(key, mode | O_CREAT | O_EXCL);
	return msqid;
}

void remove_msq(int msqid)
{
	msgctl(msqid, IPC_RMID, NULL);
}

int open_msq(uint32_t key, mode_t mode)
{
	int msqid;

	msqid = msgget(key, mode);
	return msqid;
}

ssize_t msq_read(int msqid, void *message, size_t nbytes)
{
	ssize_t ncount;

	ncount = msgrcv(msqid, message, nbytes, *(long *)message, 0);
	return ncount;
}

ssize_t msq_write(int msqid, void *message, size_t nbytes)
{
	ssize_t ncount;

	ncount = msgsnd(msqid, message, nbytes, 0);
	return ncount;
}

void *create_message(long type, size_t nbytes)
{
	void *message;

	message = zmalloc(nbytes + sizeof(long));
	*((long *)message) = type;
	return message;
}
