#include <stdlib.h>
#include "client.h"
#include "log.h"

int main()
{
	int fd;

	log_init("", LOG_DEBUG);
	if(init_client() < 0)
		exit(1);
	fd = f_open("/temp/a.txt", CREATE_TEMP | RDWR, RUSR | WUSR);
	if(fd < 0)
	{
		log_write(LOG_ERR, "open file failed");
		exit(1);
	}
	log_write(LOG_DEBUG, "file open successfully");
	f_close(fd);
	return 0;
}
