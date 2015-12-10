#include <stdio.h>
#include "zmalloc.h"
#include "msg_ipc.h"
#define KEY 8713

int main()
{
	void *msg, *msg_r;
	int i, msqid;

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
	remove_msq(msqid);
	zfree(msg);
	zfree(msg_r);
	return 0;
}
