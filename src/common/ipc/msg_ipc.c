#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/msg.h>
#include <string.h>
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

ssize_t m_read(int msqid, void *buff, size_t nbytes)
{
	ssize_t ncount;
	void *message;

	message = create_message(COMMON_TYPE, nbytes);
	ncount = msq_read(msqid, message, nbytes);
	memcpy(buff, MSG_TEXT_PTR(message), nbytes);
	zfree(message);
	return ncount;
}

ssize_t m_write(int msqid, void *buff, size_t nbytes)
{
	ssize_t ncount;
	void *message;

	message = create_message(COMMON_TYPE, nbytes);
	memcpy(MSG_TEXT_PTR(message), buff, nbytes);
	ncount = msq_write(msqid, message, nbytes);
	zfree(message);
	return ncount;
}

void *create_message(long type, size_t nbytes)
{
	void *message;

	message = zmalloc(nbytes + sizeof(long));
	*((long *)message) = type;
	return message;
}
