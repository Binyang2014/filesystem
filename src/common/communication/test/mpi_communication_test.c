#include <stdio.h>
#include "../log.h"
#include "../mpi_communication.h"

int main(int argc, char* argv[])
{
	int id;
	char send_buff[20], recv_buff[20];

	log_init("", LOG_DEBUG);
	mpi_init_with_thread(&argc, &argv);
	id = get_mpi_rank();
	if(id != 0)
	{
		mpi_recv(recv_buff, 0, -1, 10, NULL);
	}
	else
	{
		int i;
		for(i = 1; i < 10; i++)
		{
			send_buff[i] = i;
		}
		mpi_send(send_buff, 1, 0, 10);
	}
	mpi_finish();
	return 0;
}
