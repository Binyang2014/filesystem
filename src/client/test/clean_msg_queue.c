#include <stdio.h>
#include "client_struct.h"
#include "msg_ipc.h"
int main()
{
	remove_msq(MSG_QUEUE_S_TO_C);
	remove_msq(MSG_QUEUE_C_TO_S);
	return 0;
}
