#include <stdio.h>
#include "zmalloc.h"
#include "msg_ipc.h"
#define KEY 8713

int main()
{
	void *msg, *msg_r;
	int i, msqid;
	char send[5], recv[5];

	msg = create_message(1, 5);
	msg_r = create_message(1, 5);
	for(i = 0; i < 5; i++)
		((char *)MSG_TEXT_PTR(msg))[i] = '0' + i;
	msqid = create_msq(KEY, 0660);
	open_msq(KEY, 0660);
	msq_write(msqid, msg, 5);
	msq_read(msqid, msg_r, 5);
	for(i = 0; i < 5; i++)
		printf("%c ", ((char *)MSG_TEXT_PTR(msg))[i]);
	printf("\n");
	
//=============================other test=======================
	for(i = 0; i < 5; i++)
		send[i] = '1' + i;
	m_write(msqid, send, sizeof(send));
	m_read(msqid, recv, sizeof(recv));
	for(i = 0; i < 5; i++)
		printf("%c ", recv[i]);
	printf("\n");
	remove_msq(msqid);
	zfree(msg);
	zfree(msg_r);
	return 0;
}
